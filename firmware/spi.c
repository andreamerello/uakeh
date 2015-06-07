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
#include <stdlib.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/dma.h>
#include "debug_printf.h"
#include "cmd.h"

#define SPI_CMD_PREFIX "SPI "
#define SPI_DMA_TX DMA_CHANNEL3
#define SPI_DMA_RX DMA_CHANNEL2
#define SPI_DMA_PRIO DMA_CCR_PL_VERY_HIGH

cmd_res_t spi_cmd_xfer(char *);
cmd_res_t spi_cmd_cfg(char *);

unsigned int spi_khz, spi_ckpol, spi_ckpha, spi_presc, spi_frame = 0;
char spi_endian;

CMD_DECLARE_LIST(spi_cmds) = {
	{
		.str = SPI_CMD_PREFIX"XFER",
		.handler = spi_cmd_xfer,
		.help = "<len> <dat #1> .. <dat #len>"
	},
	{
		.str = SPI_CMD_PREFIX"CFG",
		.handler = spi_cmd_cfg,
		.help = "<KHz> <endian l/b> <frame 8/16> <ckpol 1/0> <chpha 1/0>"
	}
};

void spi_set_dma_size(uint32_t size)
{
	uint32_t msize, psize;

	if (size == 8) {
		msize = DMA_CCR_MSIZE_8BIT;
		psize = DMA_CCR_PSIZE_8BIT;
	} else {
		msize = DMA_CCR_MSIZE_16BIT;
		psize = DMA_CCR_PSIZE_16BIT;
	}
	dma_set_memory_size(DMA1, SPI_DMA_TX, msize);
	dma_set_memory_size(DMA1, SPI_DMA_RX, msize);
	dma_set_peripheral_size(DMA1, SPI_DMA_RX, psize);
	dma_set_peripheral_size(DMA1, SPI_DMA_RX, psize);
}

cmd_res_t spi_cmd_cfg(char *str)
{
	unsigned int khz, endian, frame, ckpol, ckpha, presc;
	int clock = rcc_apb2_frequency / 1000;

	if (5 != sscanf(str, "%u %c %u %u %u",
				&khz, &endian, &frame, &ckpol, &ckpha))
		return CMD_ERR;
	if ((ckpol != !!ckpol) || (ckpha != !!ckpha))
		return CMD_ERR;
	if ((endian != 'L') && (endian != 'B'))
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

	spi_disable(SPI1);
	spi_set_baudrate_prescaler(SPI1, presc);

	if (endian == 'l')
		spi_send_lsb_first(SPI1);
	else
		spi_send_msb_first(SPI1);

	spi_set_dma_size(frame);
	if (frame == 8) {
		spi_set_dff_8bit(SPI1);
	} else {
		spi_set_dff_16bit(SPI1);
	}

	if (ckpol)
		spi_set_clock_polarity_1(SPI1);
	else
		spi_set_clock_polarity_0(SPI1);

	if (ckpha)
		spi_set_clock_phase_1(SPI1);
	else
		spi_set_clock_phase_0(SPI1);

	spi_enable(SPI1);
	return CMD_OK;
}

cmd_res_t spi_cmd_xfer(char *str)
{
	int idx, sz, ascii_sz, val, val_mask;
	unsigned int i, len;
	void *rbuf, *tbuf, *tmpbuf;
	char *abuf;
	const char *rx_format = (spi_frame == 8) ? "0x%02x " : "0x%04x ";

	if (spi_frame == 0) {
		cmd_send("Err: SPI not configured!");
		return CMD_SILENT;
	}
	sz = (spi_frame == 8) ? 1 : 2;
	/* Worst case ascii len for response.
	 * Take in account "0x" prefix, space at end, and digits (2 x bytes)
	 */
	ascii_sz = 3 + sz * 2;
	val_mask = (1 << spi_frame) - 1;
	if (1 != sscanf(str, "%u %n", &len, &idx))
		return CMD_ERR;

	rbuf = alloca(len * sz);
	/* to save memory, allocate only one buffer that can
	 * carry the TX payload and the ascii temp buffer
	 * for processing RX data. (the latter is certainly
	 * not smaller).
	 */
	tmpbuf = alloca(len * ascii_sz + 1);
	abuf = (char*)(tbuf = tmpbuf);
	str += idx;

	for (i = 0; i < len; i++) {
		if (1 != sscanf(str, "%x %n", &val, &idx))
			return CMD_ERR;
		if (val & ~val_mask)
			return CMD_ERR;
	        memcpy(tbuf + i * sz, &val, sz);
		str += idx;
	}

	dma_set_number_of_data(DMA1, SPI_DMA_TX, len);
	dma_set_number_of_data(DMA1, SPI_DMA_RX, len);

	dma_set_memory_address(DMA1, SPI_DMA_TX, (uint32_t)tbuf);
	dma_set_memory_address(DMA1, SPI_DMA_RX, (uint32_t)rbuf);

	dma_enable_channel(DMA1, SPI_DMA_RX);
	dma_enable_channel(DMA1, SPI_DMA_TX);
	spi_enable_rx_dma(SPI1);
	spi_enable_tx_dma(SPI1);

	while(!dma_get_interrupt_flag(DMA1, SPI_DMA_RX, DMA_TCIF));

	dma_disable_channel(DMA1, SPI_DMA_TX);
	dma_disable_channel(DMA1, SPI_DMA_RX);

	idx = 0;
	for (i = 0; i < len; i++) {
		if (spi_frame == 8) {
			val = *(uint8_t*)(rbuf + i);
		} else {
			val = *(uint16_t*)(rbuf + i * 2);
		}
		idx += sprintf(abuf + idx, rx_format, val);
	}

	cmd_send(abuf);

	return CMD_SILENT;
}

void spi_init()
{
	CMD_REGISTER_LIST(spi_cmds);

	rcc_periph_clock_enable(RCC_SPI1);
	rcc_periph_clock_enable(RCC_DMA1);

	spi_reset(SPI1);
	spi_set_master_mode(SPI1);
	spi_enable_ss_output(SPI1);

	dma_channel_reset(DMA1, SPI_DMA_TX);
	dma_channel_reset(DMA1, SPI_DMA_RX);
	dma_enable_memory_increment_mode(DMA1, SPI_DMA_TX);
	dma_enable_memory_increment_mode(DMA1, SPI_DMA_RX);
	dma_disable_peripheral_increment_mode(DMA1, SPI_DMA_TX);
	dma_disable_peripheral_increment_mode(DMA1, SPI_DMA_RX);
	dma_set_peripheral_address(DMA1, SPI_DMA_TX, (uint32_t)&SPI1_DR);
	dma_set_peripheral_address(DMA1, SPI_DMA_RX, (uint32_t)&SPI1_DR);
	dma_set_read_from_memory(DMA1, SPI_DMA_TX);
	dma_set_read_from_peripheral(DMA1, SPI_DMA_RX);
	dma_set_priority(DMA1, SPI_DMA_RX, SPI_DMA_PRIO);
	dma_set_priority(DMA1, SPI_DMA_TX, SPI_DMA_PRIO);
}
