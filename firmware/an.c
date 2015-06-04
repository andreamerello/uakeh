/*
 * This file is part of UAKEH (Usb Army Knife for Electronic Hacks) project.
 * Copyright (C) 2015 Andrea Merello <andrea.merello@gmail.com>
 *
 *  Partially based on adc.c in "adc_temperature_sensor" example in
 *  in libopencm3-examples
 *  Copyright (C) 2010 Thomas Otto <tommi@viadmin.org>
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
#include <libopencm3/stm32/adc.h>
#include "debug_printf.h"
#include "cmd.h"

#define AN_CMD_PREFIX "AN "

cmd_res_t an_cmd_read(char *);

CMD_DECLARE_LIST(an_cmds) = {
	{
		.str = AN_CMD_PREFIX"RD",
		.handler = an_cmd_read,
		.help = "<ANx>"
	}
};

cmd_res_t an_cmd_read(char *str)
{
	uint8_t channel_array[16];
	int ch;
	unsigned long res;
	char s_res[8];

	if (1 != sscanf(str, "%d", &ch))
		return CMD_ERR;

	channel_array[0] = ch;
	adc_set_regular_sequence(ADC1, 1, channel_array);

	/*
	 * Start the conversion directly (not trigger mode).
	 */
	adc_start_conversion_direct(ADC1);
	while (!adc_eoc(ADC1));
	res = adc_read_regular(ADC1);
	res *= 3300;
	res /= 4096;
	sprintf(s_res, "%d mV", res);
	cmd_send(s_res);
	return CMD_SILENT;
}


void an_init()
{
	int i;
	CMD_REGISTER_LIST(an_cmds);

	rcc_periph_clock_enable(RCC_ADC1);

	/* Make sure the ADC doesn't run during config. */
	adc_off(ADC1);

	/* We configure everything for one single conversion. */
	adc_disable_scan_mode(ADC1);
	adc_set_single_conversion_mode(ADC1);
	adc_disable_external_trigger_regular(ADC1);
	adc_set_right_aligned(ADC1);

	adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_28DOT5CYC);

	adc_power_on(ADC1);

	/* Wait for ADC starting up. */
	for (i = 0; i < 800000; i++)    /* Wait a bit. */
		__asm__("nop");

	adc_reset_calibration(ADC1);
	adc_calibration(ADC1);
}
