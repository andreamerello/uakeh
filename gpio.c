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

#include <stdio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "debug_printf.h"
#include "cmd.h"

#define GP_CMD_PREFIX "GP "

void gpio_cmd_set_cfg(char *);
void gpio_cmd_get_cfg(char *);
void gpio_cmd_read(char *);
void gpio_cmd_set(char *);

typedef struct {
	char str[8];
	uint8_t val;
} gpio_arg_t;

typedef struct {
	char str[8];
	uint32_t val;
} gpio_arg32_t;

static gpio_arg_t arg_drv[] = {
	{.str = "OD", .val = GPIO_CNF_OUTPUT_OPENDRAIN},
	{.str = "OC", .val = GPIO_CNF_OUTPUT_OPENDRAIN},
	{.str = "PP", .val = GPIO_CNF_OUTPUT_PUSHPULL},
	{.str = "TP", .val = GPIO_CNF_OUTPUT_PUSHPULL},
};

static gpio_arg_t arg_slope[] = {
	{.str = "2MHZ", .val = GPIO_MODE_OUTPUT_2_MHZ},
	{.str = "2", .val = GPIO_MODE_OUTPUT_2_MHZ},
	{.str = "10MHZ", .val = GPIO_MODE_OUTPUT_10_MHZ},
	{.str = "10", .val = GPIO_MODE_OUTPUT_10_MHZ},
	{.str = "50MHZ", .val = GPIO_MODE_OUTPUT_50_MHZ},
	{.str = "50", .val = GPIO_MODE_OUTPUT_50_MHZ},
};

static gpio_arg_t arg_pull[] = {
	{.str = "FLOAT", .val = GPIO_CNF_INPUT_FLOAT},
	{.str = "NONE", .val = GPIO_CNF_INPUT_FLOAT},
	{.str = "PUP", .val = GPIO_CNF_INPUT_PULL_UPDOWN},
	{.str = "PDN", .val = GPIO_CNF_INPUT_PULL_UPDOWN},
	{.str = "PD", .val = GPIO_CNF_INPUT_PULL_UPDOWN},
	{.str = "PN", .val = GPIO_CNF_INPUT_PULL_UPDOWN},
};

static gpio_arg32_t arg_port[] = {
	{.str = "PA", .val = GPIOA},
	{.str = "PORTA", .val = GPIOA},
	{.str = "A", .val = GPIOA},
	{.str = "GPIOA", .val = GPIOA},

	{.str = "PB", .val = GPIOB},
	{.str = "PORTB", .val = GPIOB},
	{.str = "B", .val = GPIOB},
	{.str = "GPIOB", .val = GPIOB},

	{.str = "PC", .val = GPIOC},
	{.str = "PORTC", .val = GPIOC},
	{.str = "C", .val = GPIOC},
	{.str = "GPIOC", .val = GPIOC},
};


CMD_DECLARE_LIST(gpio_cmds) = {
	{ .str = GP_CMD_PREFIX"SETCFG", .handler = gpio_cmd_set_cfg },
	{ .str = GP_CMD_PREFIX"GETCFG", .handler = gpio_cmd_get_cfg },
	{ .str = GP_CMD_PREFIX"RD", .handler = gpio_cmd_read },
	{ .str = GP_CMD_PREFIX"WR", .handler = gpio_cmd_set },
};

#define gpio_arg(a, b, c) __gpio_arg(a, sizeof(a) / sizeof(a[0]), b, c)
int __gpio_arg(gpio_arg_t *list, int len, char *str, uint8_t *val)
{
	int i;
	gpio_arg_t *arg;
	for (i = 0; i < len; i++) {
		arg = list + i;
		if (strcmp(str, arg->str) == 0) {
			*val = arg->val;
			return 0;
		}
	}
	return -1;
}

#define gpio_arg32(a, b, c) __gpio_arg32(a, sizeof(a) / sizeof(a[0]), b, c)
int __gpio_arg32(gpio_arg32_t *list, int len, char *str, uint32_t *val)
{
	int i;
	gpio_arg32_t *arg;
	for (i = 0; i < len; i++) {
		arg = list + i;
		if (strcmp(str, arg->str) == 0) {
			*val = arg->val;
			return 0;
		}
	}
	return -1;
}

int gpio_pins(unsigned int pin, uint16_t *pins)
{
	const uint16_t map[] = {
		GPIO0, GPIO1, GPIO2, GPIO3, GPIO4, GPIO5, GPIO6, GPIO7,
		GPIO8, GPIO9, GPIO10, GPIO11, GPIO12, GPIO13, GPIO14, GPIO15 };
	if (pin > (sizeof(map) / sizeof(map[0])))
		return -1;
	*pins = map[pin];

	return 0;
}

int gpio_port_pin(char *str, uint32_t *port, int *pin, int *idx)
{
	char s_port[8];

	if (2 == sscanf(str, "%8s %u %n", &s_port, pin, idx))
		return -1;
	if (gpio_arg32(arg_port, s_port, port) < 0)
		return -2;
	return 0;
}

void gpio_cmd_set_cfg(char *str)
{

	char s_drv[4];
	char s_slope[8];
	char s_pull[8];
	int pin;
	uint32_t port;
	uint16_t pins;
	uint8_t pull, slope, drv;
	int idx;

	if (gpio_port_pin(str, &port, &pin, &idx))
		goto err;
	if (gpio_pins(pin, &pins) < 0)
		goto err;

	str += idx;

	if (1 == sscanf(str, "IN %8s", &s_pull)) {
		if (gpio_arg(arg_pull, s_pull, &pull))
			goto err;
		gpio_set_mode(port, GPIO_MODE_INPUT, pull, pins);
	} else if (2 == sscanf(str,"OUT %4s %8s", &s_drv, &s_slope)) {
		if (gpio_arg(arg_slope, s_slope, &slope) < 0)
			goto err;
		if (gpio_arg(arg_drv, s_drv, &drv) < 0)
			goto err;
		gpio_set_mode(port, slope, drv, pins);
	} else {
	}

	return;
err:
	cmd_send("error: wrong params. Use:");
	cmd_send(GP_CMD_PREFIX"SETCFG <PORT> <PIN> IN <PUP/PDN/NONE>");
	cmd_send(GP_CMD_PREFIX"SETCFG <PORT> <PIN> OUT <OD/PP> <2MHZ/10MHZ/50MHZ>");
}

void gpio_cmd_set(char *str)
{
	int on;
 	uint32_t port;
	uint16_t pins;
	int pin;
	int idx;

	if (gpio_port_pin(str, &port, &pin, &idx))
		goto err;
	if (gpio_pins(pin, &pins) < 0)
		goto err;

	str += idx;
	sscanf(str, "%d", &on);

	if (on)
		gpio_set(port, pins);
	else
		gpio_clear(port, pins);

	return;

err:
	cmd_send("error: wrong params. Use:");
	cmd_send(GP_CMD_PREFIX"WR <PORT> <PIN> <1/0>");
}

void gpio_cmd_get_cfg(char *str)
{
}

void gpio_cmd_read(char *str)
{
	int on;
	uint32_t port;
	int pin;
	int idx;

	if (gpio_port_pin(str, &port, &pin, &idx)) {
		cmd_send("error: wrong params. Use:");
		cmd_send(GP_CMD_PREFIX"RD <PORT> <PIN>");
		return;
	}

	on = gpio_port_read(port) & pin;

	cmd_send(on ? "1" : "0");
}


void gpio_init()
{
	CMD_REGISTER_LIST(gpio_cmds);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
}
