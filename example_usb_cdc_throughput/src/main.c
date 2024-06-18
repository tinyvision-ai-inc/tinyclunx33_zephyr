#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/drivers/usb/udc.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/usb/bos.h>
#include <zephyr/usb/class/usb_cdc_raw.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* Grab the devices from the devicetree */
#define UDC0 DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0))
#define CDC0 DEVICE_DT_GET(DT_NODELABEL(cdc0))
#define PLL0 DEVICE_DT_GET(DT_NODELABEL(pll0))
#define RAM1_ADDR DT_REG_ADDR(DT_NODELABEL(ram1))
#define RAM1_SIZE DT_REG_SIZE(DT_NODELABEL(ram1))

/* USB descriptors and other definitions */
USBD_DEVICE_DEFINE(my_usbd, UDC0, 0x1209, 0x0001);
USBD_DEVICE_QUALIFIER_DEFINE(my_usbd_dev_qualifier);
USBD_CONFIGURATION_DEFINE(my_usbd_config, USB_SCD_SELF_POWERED, 100);
USBD_BOS_DEFINE(my_usbd_bos);
USBD_DESC_LANG_DEFINE(my_usbd_lang);
USBD_DESC_MANUFACTURER_DEFINE(my_usbd_manufacturer, "tinyVision.ai");
USBD_DESC_PRODUCT_DEFINE(my_usbd_product, "tinyCLUNX33");
//USBD_DESC_SERIAL_NUMBER_DEFINE(my_usbd_serial_number, "0123456789ABCDEF");

/* Static pool of buffer with no data in it */
NET_BUF_POOL_DEFINE(_buf_pool, 100, 0, sizeof(struct udc_buf_info), NULL);

static int _write_callback(const struct device *dev, struct net_buf *buf, int err)
{
	struct udc_buf_info *bi = udc_get_buf_info(buf);

	if (err) {
		LOG_ERR("USB write error %d", err);
		return err;
	}
	LOG_DBG("%s: buf=%p data=%p len=%d size=%d zlp=%d", __func__,
		buf, buf->data, buf->len, buf->size, bi->zlp);

	/* Freeing here will unblock the memory pool */
	net_buf_unref(buf);
	return 0;
}

void sleep_ms(uint32_t msec)
{
	for (int ms = 0; ms < msec; ms += 20) {
		k_sleep(K_MSEC(10));
		usb23_irq_handler(UDC0);
	}
}

int main(void)
{
	struct net_buf *buf;
	int err;

	k_sleep(K_MSEC(200));
	clock_control_on(PLL0, NULL);
	k_sleep(K_MSEC(200));

	err = usbd_add_descriptor(&my_usbd, &my_usbd_dev_qualifier);
	err |= usbd_add_descriptor(&my_usbd, &my_usbd_bos);
	err |= usbd_add_descriptor(&my_usbd, &my_usbd_lang);
	err |= usbd_add_descriptor(&my_usbd, &my_usbd_manufacturer);
	err |= usbd_add_descriptor(&my_usbd, &my_usbd_product);
	//err |= usbd_add_descriptor(&my_usbd, &my_usbd_serial_number);
	err |= usbd_add_configuration(&my_usbd, &my_usbd_config);
	err |= usbd_register_class(&my_usbd, "cdc_raw_0", 1);
	__ASSERT_NO_MSG(err == 0);

	err = usbd_init(&my_usbd);
	__ASSERT_NO_MSG(err == 0);

	err = usbd_enable(&my_usbd);
	__ASSERT_NO_MSG(err == 0);

	/* Hook the callback called used once a write is completed */
	cdc_raw_set_write_callback(CDC0, &_write_callback);

	/* Wait that a serial console connects */
	while (!cdc_raw_is_ready(CDC0)) {
		sleep_ms(100);
	}

	for (int i = 3; i > 0; i--) {
		LOG_INF("sending something in %d", i);
		sleep_ms(1000);
	}

	LOG_INF("Starting the bulk throughput test...");
	while (true) {
		/* This will wait until _write_callback frees some more buffers */
		buf = net_buf_alloc_with_data(&_buf_pool, (void *)RAM1_ADDR, RAM1_ADDR, K_NO_WAIT);
		if (buf != NULL) {
			cdc_raw_write(CDC0, buf, false);
		}

		sleep_ms(10);
	}

	return 0;
}