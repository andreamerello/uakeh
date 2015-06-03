/*
 * This file is part of UAKEH (Usb Army Knife for Electronic Hacks) project.
 * Copyright (C) 2015 Andrea Merello <andrea.merello@gmail.com>
 *
 *  Partially based on "pwm_6stem.c" libopencm3-examples
 *  Copyright (C) 2011 Piotr Esden-Tempski <piotr@esden.net>
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
#include <libopencm3/stm32/timer.h>
#include "debug_printf.h"
#include "cmd.h"

#define AN_CMD_PREFIX "PWM "

cmd_res_t pwm_cmd_set_frq(char *str);
cmd_res_t pwm_cmd_set_perc(char *str);
cmd_res_t pwm_cmd_enable(char *str);

/* for R/C servo and ESC */
static int pwm_rc_mode = 0;
static int pwm_max_period;

CMD_DECLARE_LIST(pwm_cmds) = {
	{
		.str = AN_CMD_PREFIX"EN",
		.handler = pwm_cmd_enable,
		.help = "<1/0>"
	},
	{
		.str = AN_CMD_PREFIX"FREQ",
		.handler = pwm_cmd_set_frq,
		.help = "[<HZ>/RC]"
	},
	{
		.str = AN_CMD_PREFIX"PERC",
		.handler = pwm_cmd_set_perc,
		.help = "<percentage>"
	},

};

void pwm_set_perc(float f_perc)
{
	unsigned long i_perc;

#warning TBD_RC-mode
	i_perc = (unsigned long)(f_perc * pwm_max_period);
	i_perc /= 100;

	timer_set_oc_value(TIM1, TIM_OC1, i_perc);
}

cmd_res_t pwm_cmd_set_perc(char *str)
{
	float f_perc;

	if (1 != sscanf(str, "%f", &f_perc))
			return CMD_ERR;
	if ((f_perc > 100) || (f_perc < 0))
		return CMD_ERR;
	pwm_set_perc(f_perc);
	return CMD_OK;
}

cmd_res_t pwm_cmd_set_frq(char *str)
{
	unsigned long freq, ratio, presc;
#warning TBD_realclock
	const unsigned long clock = 48000000;

	if (strcmp(str, "RC") == 0) {
		pwm_rc_mode = 1;
		freq = 50;
	} else {
		pwm_rc_mode = 0;
		if (1 != sscanf(str, "%lu", &freq))
			return CMD_ERR;
	}

	if (freq > clock)
		return CMD_ERR;

	/* final ratio */
	ratio = clock / freq;
	presc = ratio / 65535;

	pwm_max_period = ratio / (presc + 1);

	timer_set_prescaler(TIM1, presc);
	timer_set_period(TIM1, pwm_max_period);

	if (pwm_rc_mode) {
		pwm_set_perc(0.0);
	} else {
		pwm_set_perc(50.0);
	}

	return CMD_OK;
}

cmd_res_t pwm_cmd_enable(char *str)
{
	unsigned int en;

	if (1 != sscanf(str, "%u", &en))
		return CMD_ERR;
	if (en != !!en)
		return CMD_ERR;

	if (en) {
		timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_PWM1);
		timer_enable_counter(TIM1);
	} else {
		timer_disable_oc_output(TIM1, TIM_OC1);
		timer_disable_counter(TIM1);
	}
	return CMD_OK;
}


void pwm_init()
{
	CMD_REGISTER_LIST(pwm_cmds);

	rcc_periph_clock_enable(RCC_TIM1);

	timer_reset(TIM1);
	timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT,
		TIM_CR1_CMS_CENTER_1, TIM_CR1_DIR_DOWN);
	timer_set_repetition_counter(TIM1, 0);

	timer_continuous_mode(TIM1);

	timer_set_enabled_off_state_in_idle_mode(TIM1);
	timer_set_disabled_off_state_in_run_mode(TIM1);
	timer_disable_break(TIM1);

	timer_disable_oc_clear(TIM1, TIM_OC1);
	timer_set_oc_slow_mode(TIM1, TIM_OC1);

	timer_set_oc_polarity_high(TIM1, TIM_OC1);
	timer_set_oc_idle_state_set(TIM1, TIM_OC1);

	timer_enable_preload(TIM1);
	timer_enable_oc_preload(TIM1, TIM_OC1);
}
