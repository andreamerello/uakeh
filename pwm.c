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

cmd_res_t pwm_cmd_xxx(char *);

CMD_DECLARE_LIST(pwm_cmds) = {
	{
		.str = AN_CMD_PREFIX"xxx",
		.handler = pwm_cmd_xxx,
		.help = "xxx"
	}
};

cmd_res_t pwm_cmd_xxx(char *str)
{
	return CMD_OK;
}


void pwm_init()
{
	CMD_REGISTER_LIST(pwm_cmds);

	rcc_periph_clock_enable(RCC_TIM1);
/*	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
		      GPIO_TIM1_CH1 );
*/
	timer_reset(TIM1);
	timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_CENTER_1, TIM_CR1_DIR_DOWN);
	timer_set_prescaler(TIM1, 12);
	timer_set_period(TIM1, 40000);
	timer_set_repetition_counter(TIM1, 0);
	timer_continuous_mode(TIM1);

	timer_set_enabled_off_state_in_idle_mode(TIM1);
	timer_set_disabled_off_state_in_run_mode(TIM1);
	timer_disable_break(TIM1);

	timer_disable_oc_clear(TIM1, TIM_OC1);
	timer_enable_oc_preload(TIM1, TIM_OC1);
	timer_set_oc_slow_mode(TIM1, TIM_OC1);
	timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_PWM1);

	timer_set_oc_polarity_high(TIM1, TIM_OC1);
	timer_set_oc_idle_state_set(TIM1, TIM_OC1);


//	timer_set_oc_value(TIM1, TIM_OC1, 1830); //1ms
//	timer_set_oc_value(TIM1, TIM_OC1, 2750); //1.5ms
	timer_set_oc_value(TIM1, TIM_OC1, 3670); //2ms

	timer_enable_preload(TIM1);
	timer_enable_oc_preload(TIM1, TIM_OC1);
	timer_enable_counter(TIM1);
}
