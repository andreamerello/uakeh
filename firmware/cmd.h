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

#ifndef __CMD_H__
#define __CMD_H__

#include <string.h>
#define CMD_MAX 64

typedef enum {
	CMD_OK,
	CMD_ERR,
	CMD_SILENT
} cmd_res_t;

typedef struct {
	char *str;
	char *help;
	cmd_res_t (*handler)(char*);
} cmd_t;

struct _cmd_set_t {
	cmd_t *set;
	int len;
	struct _cmd_set_t *next;
};

typedef struct _cmd_set_t cmd_set_t;

#define TOSET(x) __##x##_set
extern void __cmd_register_set(cmd_set_t *set);

/* public API follows */
#define CMD_DECLARE_LIST(x) \
	static cmd_set_t TOSET(x); \
	cmd_t x[]

#define CMD_REGISTER_LIST(x) \
	__cmd_register_set(&TOSET(x));\
	TOSET(x).set = x; \
	TOSET(x).next = NULL; \
	TOSET(x).len = sizeof(x) / sizeof(x[0]);

extern void cmd_init(void);
extern void cmd_poll(void);
extern void cmd_send(char *buf);
#endif
