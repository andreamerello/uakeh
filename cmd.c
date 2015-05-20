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

static void cmd_dispatch(char *cmdstr)
{
	cmd_set_t *set;
	cmd_t *tmp_cmd;
	unsigned int cmdlen;
	int i;

	cmd_trim(cmdstr);

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
				tmp_cmd->handler(cmdstr + cmdlen);
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
#ifdef CMD_DEBUG_RX1
	printf(".%x\n", data[0]);
#endif
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

void cmd_init()
{
	usb_dev = cdcacm_init();
	cdcacm_register_rx_cb(cmd_parse);
}
