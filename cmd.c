#include "cmd.h"
#include "cdcacm.h"

#define CMD_LF 0xa

static cmd_set_t *cmd_set = NULL;
static usbd_device *usb_dev;

void __cmd_register_set(cmd_set_t *set)
{
	cmd_set_t *set_tmp;

	if (cmd_set == NULL) {
		cmd_set = set;
	} else {
		for (set_tmp = cmd_set;
		     set_tmp->next;
		     set_tmp = set_tmp->next);

		set_tmp->next = set;
	}
	set->next = NULL;
}

static void cmd_dispatch(char *cmdstr)
{
	cmd_set_t *set;
	cmd_t *tmp_cmd;
	int i;

	for (set = cmd_set; set; set = set->next) {
		for (i = 0; i < set->len; i++) {
			tmp_cmd = &set->set[i];
			if (0 == strncmp(cmdstr, tmp_cmd->str, strlen(cmdstr))) {
				tmp_cmd->handler(cmdstr - strlen(cmdstr));
				return;
			}
		}
	}
	cmd_send("Unknown command :(");
}

static void cmd_parse(char *data, int len)
{
	int i;
	char ch;
	static char cmd_buf[CMD_MAX];
	static int cmd_len = 0;
cdcacm_tx("r",1);
return;
	for (i = 0; i < len; i++) {
		ch = data[i];

		if (cmd_len == 0 && ch == ' ')
			continue;

		if (ch == CMD_LF) {
			cmd_buf[cmd_len] = '\0';
			cmd_dispatch(cmd_buf);
			cmd_len = 0;
		} else {
			cmd_buf[cmd_len] = ch;
			cmd_len++;
		}
	}
}

void cmd_send(char *buf)
{
	int len;

	len = strlen(buf);
	buf[len] = CMD_LF;
	cdcacm_tx(buf, len);
}

void cmd_poll()
{
	usbd_poll(usb_dev);
}

void cmd_init()
{
	usb_dev = cdcacm_init();
	cdcacm_register_rx_cb(cmd_parse);
}
