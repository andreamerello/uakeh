#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "cdcacm.h"

void cmd_rx(char buf[64], int len)
{
	cdcacm_tx(buf, len);
}

int main(void)
{
	int i;

	usbd_device *usbd_dev;

	rcc_clock_setup_in_hsi_out_48mhz();

	usbd_dev = cdcacm_init();
	cdcacm_register_rx_cb(cmd_rx);

//	rcc_periph_clock_enable(RCC_GPIOC);

//	gpio_set(GPIOC, GPIO11);
//	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
//		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO11);


//	for (i = 0; i < 0x800000; i++)
//		__asm__("nop");
//	gpio_clear(GPIOC, GPIO11);

	while (1)
		usbd_poll(usbd_dev);
}
