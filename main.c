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
#include "gpio.h"
#include "an.h"

#define FW_VERSION "V 0.1"

/*     PIN MAPPING
 *
 * PC13: GPIO
 * PC14: GPIO
 * PC15: GPIO
 * PA0:  GPIO/AN0
 * PA1:  GPIO/AN1
 * PA2:  GPIO/AN2
 * PA3:  GPIO/AN3
 * PA4:  GPIO/AN4/SPI1_CS
 * PA5:  GPIO/AN5/SPI1_CK
 * PA6:  GPIO/AN6/SPI1_MISO
 * PA7:  GPIO/AN7/SPI1_MOSI
 * PA8:  GPIO
 * PA9:  GPIO
 * PA10: GPIO
 * PA11: USBDM (rsvd)
 * PA12: USBDP (rsvd)
 * PA13: GPIO(/JTMS/SWDIO)
 * PA14: GPIO(/JTCK/SWCLK)
 * PA15: GPIO/SPI1_CS(/JTDI)
 * PB0:  GPIO/AN8
 * PB1:  GPIO/AN9
 * PB2:  BOOT (rsvd)?
 * PB3:  GPIO/SPI1_SCK(/JTDO)
 * PB4:  GPIO/SPI1_MISO(/JNTRST)
 * PB5:  GPIO/SPI1_MOSI
 * PB6:  GPIO/I2C1_SCL
 * PB7:  GPIO/I2C1_SDA
 * PB8:  GPIO/CANRX
 * PB9:  GPIO/CANTX
 * PB10: GPIO/I2C2_SCL
 * PB11: GPIO/I2C2_SDA
 * PB12: GPIO/SPI2_CS
 * PB13: GPIO/SPI2_CK
 * PB14: GPIO/SPI2_MISO
 * PB15: GPIO/SPI2_MOSI
 */

cmd_res_t cmd_fwv(char *args);
cmd_res_t cmd_lic(char *args);
void init_modules(void);

CMD_DECLARE_LIST(main_cmds) = {
	{ .str = "FWV", .handler = cmd_fwv, .help = NULL },
	{ .str = "LIC", .handler = cmd_lic, .help = NULL }
};

cmd_res_t cmd_fwv(char *args)
{
	cmd_send(FW_VERSION);
	return CMD_SILENT;
}

cmd_res_t cmd_lic(char *args)
{
	cmd_send("GPL");
	return CMD_SILENT;
}

void init_modules()
{
	gpio_init();
	an_init();
}

int main(void)
{
	int i;

	rcc_clock_setup_in_hsi_out_48mhz();
	debug_init();
	printf("Device UP!\nFW version %s\n", FW_VERSION);

	cmd_init();

	CMD_REGISTER_LIST(main_cmds);
	init_modules();
	printf("Init complete..\n");
	while (1)
		cmd_poll();
}
