/*
 * This file is part of UAKEH (Usb Army Knife for Electronic Hacks) project.
 * Copyright (C) 2015 Andrea Merello <andrea.merello@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cmd.h"
#include "cdcacm.h"
#include "ram.h"
#include <stdio.h>
#include "debug_printf.h"
#include <alloca.h>

//#define CMD_DEBUG_RX2
//#define CMD_DEBUG_RX1
//#define CMD_DEBUG_TX1
//#define CMD_DEBUG_TX2
//#define CMD_DEBUG_PARSE

#define CMD_LF 0xa
#define CMD_CR 0xd

static cmd_set_t *cmd_set = NULL;
static usbd_device *usb_dev;

static cmd_res_t cmd_echo(char *);
static cmd_res_t cmd_help(char *);
static int cmd_echo_en = 1;

CMD_DECLARE_LIST(cmd_cmds) = {
	{ .str = "ECHO", .handler = cmd_echo, .help = "<1/0>" },
	{ .str = "HELP", .handler = cmd_help, .help = NULL }
};

void __cmd_register_set(cmd_set_t *set)
{
	cmd_set_t *set_tmp;
#ifdef DEBUG_CMD_REG
	printf("registering command set. len %d\n", set->len);
#endif
	if (cmd_set == NULL) {
		cmd_set = set;
	} else {
		for (set_tmp = cmd_set;
		     set_tmp->next;
		     set_tmp = set_tmp->next);

		set_tmp->next = set;
	}
}

static void cmd_trim(char *str)
{
	/* the cmd arrives already stripped of leading spaces */
	int len = strlen(str);

	while (str[0] != '\0') {
		if ((str[0] == ' ') &&
			((str[1] == ' ' || str[1] == '\0'))) {
			memmove(str, str + 1, len);
			len--;
		}
		str++;
	}
}

static void cmd_do_help(cmd_t *tmp_cmd)
{
	char *help_str;

	cmd_send("Command error");
	if (tmp_cmd->help) {
		cmd_send("Usage:");
		help_str = alloca(strlen(tmp_cmd->help) +
				strlen(tmp_cmd->str) + 2);
		strcpy(help_str, tmp_cmd->str);
		strcat(help_str, " ");
		strcat(help_str, tmp_cmd->help);
		cmd_send(help_str);
	}
}

static void cmd_toupper(char *str)
{
	unsigned int i;
	const int offset = 'a' - 'A';

	for (i = 0; i < strlen(str); i++)
		if (str[i] >= 'a' && str[i] <= 'z')
			str[i] -= offset;
}

static void cmd_dispatch(char *cmdstr)
{
	cmd_set_t *set;
	cmd_t *tmp_cmd;
	unsigned int cmdlen;
	int ret;
	int i;

	cmd_trim(cmdstr);
	cmd_toupper(cmdstr);

	for (set = cmd_set; set; set = set->next) {
		for (i = 0; i < set->len; i++) {
			tmp_cmd = &set->set[i];
			cmdlen = strlen(tmp_cmd->str);

			/* if the received string is shorter than the cmd, then
			 * don't even bother going on.
			 */
			if (cmdlen > strlen(cmdstr))
				continue;

			/* if the reiceved cmd is longer than the cmd, then
			 * check whether this is due to arguments following,
			 * otherwise the command is not really matching.
			 */
			if ((cmdlen < strlen(cmdstr)) && (cmdstr[cmdlen] != ' '))
				continue;
#ifdef CMD_DEBUG_PARSE
			printf("comparing recv (%s) with cmd #%d (%s)\n",
				cmdstr, i, tmp_cmd->str);
#endif
			/* compare only up to the arguments (if any) */
			if (0 == strncmp(cmdstr, tmp_cmd->str, cmdlen)) {
				/* if the cmd has further args, then remove the
				 * (unique, since we did cmd_trim() already)
				 * space leading the args (otherwise we simply
				 * have \0 here).
				 */
				if (cmdstr[cmdlen] == ' ')
					cmdlen++;
#ifdef CMD_DEBUG_PARSE
				printf("Calling cmd handler with args:\n%s\n",
					cmdstr + cmdlen);
#endif
				ret = tmp_cmd->handler(cmdstr + cmdlen);
				if (ret == CMD_ERR)
					cmd_do_help(tmp_cmd);
				else if(ret == CMD_OK)
					cmd_send("OK!");
				return;
			}
		}
	}
	cmd_send("Unknown command :(\nyou may type 'HELP'..");
}

/* cuts the current data buffer where needed and returns the
 * number of characters to cut off from the command buffer
 * before appending data to it.
 */
static int cmd_backspace(char *data, int *_len)
{
	int i, len;
	int buf_shorten = 0;

	len = *_len;
	for (i = 0; i < len; i++) {
		/* got a backspace */
		if (data[i] == '\b') {
			/* if it's the first char */
			if (i == 0) {
				/* throw it away */
				len--;
				memmove(data, data + 1, len);
				/* and shorten the command buffer by one */
				buf_shorten++;
			} else {
				/* if it's in the middle of the data we
				 * have to move the tail over the BS itslef
				 * and the deleted char
				 */
				if ((i + 1) != len) {
					memmove(data + i - 1, data + i + 1,
						len - (i + 1));
				}
				/* throw away BS and prev char */
				len -= 2;
			}
		}
	}
	*_len = len;
	return buf_shorten;
}

static void cmd_parse(char *data, int len)
{
	int i;
	char ch;
	int shorten;
	static char cmd_buf[CMD_MAX];
	static int cmd_len = 0;
#ifdef CMD_DEBUG_RX1
	printf(".%x\n", data);
#endif


	shorten = cmd_backspace(data, &len);
	shorten = (cmd_len > shorten) ? shorten : cmd_len;
	cmd_len -= shorten;

	for (i = 0; i < shorten; i++) {
		cdcacm_tx("\b \b",  3);
	}

	if (cmd_echo_en)
		cdcacm_tx(data, len);

	for (i = 0; i < len; i++) {
		ch = data[i];

		if (cmd_len == 0 &&
			(ch == ' ' || ch == CMD_LF || ch == CMD_CR))
			continue;

		if (ch == CMD_LF || ch == CMD_CR) {
			cmd_buf[cmd_len] = '\0';
#ifdef CMD_DEBUG_RX2
			printf("recv len %d, %s\n", cmd_len, cmd_buf);
#endif
			cmd_dispatch(cmd_buf);
			cmd_len = 0;
		} else {
			if ((cmd_len + 1) == CMD_MAX)
				continue;

			cmd_buf[cmd_len] = ch;
			cmd_len++;
		}
	}
}

void cmd_send(char *buf)
{
	int len;
	char *ram_buf;

	len = strlen(buf);

#ifdef CMD_DEBUG_TX1
	printf("send %p (%s), len %d\n", buf, buf, len);
#endif
	if (IS_RAM(buf)) {
#ifdef CMD_DEBUG_TX2
		printf("RAM: send in place\n");
#endif
		ram_buf = buf;
	} else {
#ifdef CMD_DEBUG_TX2
		printf("ROM\n");
#endif
		ram_buf = alloca(len + 1);
#ifdef CMD_DEBUG_TX2
		printf("copy to %x\n", ram_buf);
#endif
		memcpy(ram_buf, buf, len);
	}
	ram_buf[len] = CMD_LF;
#ifdef CMD_DEBUG_TX2
	printf("send to cdc\n");
#endif
	cdcacm_tx(ram_buf, len + 1);
}

void cmd_poll()
{
	usbd_poll(usb_dev);
}

cmd_res_t cmd_echo(char *args)
{
	int ret;

	ret = sscanf(args, "%d", &cmd_echo_en);
	return (ret == 1) ? CMD_OK : CMD_ERR;
}

cmd_res_t cmd_help(char *args)
{
	cmd_set_t *set;
	int i;
	args;
	cmd_send("Commands:\n");
	for (set = cmd_set; set; set = set->next) {
		for (i = 0; i < set->len; i++) {
			cmd_send(set->set[i].str);
		}
	}
	return CMD_SILENT;
}

void cmd_init()
{
	CMD_REGISTER_LIST(cmd_cmds);

	usb_dev = cdcacm_init();
	cdcacm_register_rx_cb(cmd_parse);
}
