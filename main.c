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
#include <libopencm3/stm32/timer.h>
#include "debug_printf.h"
#include "cmd.h"
#include "gpio.h"
#include "an.h"
#include "spi.h"

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
 * PA8:  GPIO/PWM
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
	spi_init();
}

void test(void)
{
	rcc_periph_clock_enable(RCC_TIM1);
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
		      GPIO_TIM1_CH1 );

	timer_reset(TIM1);
	timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_set_prescaler(TIM1, 10);
	timer_set_period(TIM1, 3000);
	timer_set_repetition_counter(TIM1, 0);
	timer_continuous_mode(TIM1);

	timer_set_deadtime(TIM1, 10);
	timer_set_enabled_off_state_in_idle_mode(TIM1);
	timer_set_enabled_off_state_in_run_mode(TIM1);
	timer_disable_break(TIM1);


	timer_disable_oc_clear(TIM1, TIM_OC1);
	timer_enable_oc_preload(TIM1, TIM_OC1);
	timer_set_oc_slow_mode(TIM1, TIM_OC1);
	timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_PWM1);

	/* Configure OC1. */
	timer_set_oc_polarity_high(TIM1, TIM_OC1);
	timer_set_oc_idle_state_set(TIM1, TIM_OC1);

	/* Set the capture compare value for OC1. */
	timer_set_oc_value(TIM1, TIM_OC1, 10000);
	timer_enable_oc_output(TIM1, TIM_OC1);

	timer_enable_preload(TIM1);

	timer_enable_counter(TIM1);

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
	test();
	while (1) {
		cmd_poll();
	}
}
