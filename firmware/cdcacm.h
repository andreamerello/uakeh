#ifndef __CDC_ACM_H__
#define __CDC_ACM_H__

#include <libopencm3/usb/usbd.h>

extern usbd_device *cdcacm_init(void);
extern void cdcacm_tx(char *buf, int len);
extern void cdcacm_register_rx_cb(void (*cb)(char[64], int));

#endif
