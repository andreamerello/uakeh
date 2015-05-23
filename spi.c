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
#include <libopencm3/stm32/spi.h>
#include "debug_printf.h"
#include "cmd.h"

#define SPI_CMD_PREFIX "SPI "

cmd_res_t spi_cmd_xfer(char *);
cmd_res_t spi_cmd_cfg(char *);

unsigned int spi_khz, spi_endian, spi_frame, spi_ckpol, spi_ckpha, spi_presc;

CMD_DECLARE_LIST(spi_cmds) = {
	{
		.str = SPI_CMD_PREFIX"XFER",
		.handler = spi_cmd_xfer,
		.help = "<len> <dat 1> .. <dat len>"
	},
	{
		.str = SPI_CMD_PREFIX"CFG",
		.handler = spi_cmd_cfg,
		.help = "<KHz> <endian l/b> <frame 8/16> <ckpol 1/0> <chpha 1/0>"
	}
};

cmd_res_t spi_cmd_cfg(char *str)
{

	unsigned int khz, endian, frame, ckpol, ckpha, presc;
	int clock = rcc_apb2_frequency / 1000;

	if (5 != sscanf(str, "%u %c %u %u %u",
				&khz, &endian, &frame, &ckpol, &ckpha))
		return CMD_ERR;
	if ((ckpol != !!ckpol) || (ckpha != !!ckpha))
		return CMD_ERR;
	if ((endian != 'l') && (endian != 'b'))
		return CMD_ERR;
	if ((frame != 8) && (frame != 16))
		return CMD_ERR;

	spi_endian = endian;
	spi_frame = frame;
	spi_ckpol = ckpol;
	spi_ckpha = ckpha;

	for (presc = 0; presc < 8; presc++) {
		spi_khz = (clock / (1 << (presc + 1)));
		if (spi_khz <= khz)
			break;
	}

	/* max pre is 7. Let the "for" loop iterate over presc == 7
	 * and touch presc = 8 to get the spi_khz updated
	 */
	if (presc == 8)
		presc--;

	spi_clean_disable(SPI1);
	spi_set_master_mode(SPI1);
	spi_enable_ss_output(SPI1);

	spi_set_baudrate_prescaler(SPI1, presc);

	if (endian == 'l')
		spi_send_lsb_first(SPI1);
	else
		spi_send_msb_first(SPI1);

	if (frame == 16)
		spi_set_dff_16bit(SPI1);
	else
		spi_set_dff_8bit(SPI1);

	if (ckpol)
		spi_set_clock_polarity_1(SPI1);
	else
		spi_set_clock_polarity_0(SPI1);

	if (ckpha)
		spi_set_clock_phase_1(SPI1);
	else
		spi_set_clock_phase_0(SPI1);

	return CMD_OK;
}

cmd_res_t spi_cmd_xfer(char *str)
{
	spi_enable(SPI1);
	return CMD_SILENT;
}

void spi_init()
{
	int i;

	CMD_REGISTER_LIST(spi_cmds);

	rcc_periph_clock_enable(RCC_SPI1);
	spi_reset(SPI1);
}
