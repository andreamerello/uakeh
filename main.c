#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "cdcacm.h"

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
