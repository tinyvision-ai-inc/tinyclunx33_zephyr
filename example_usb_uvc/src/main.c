#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/usb/udc.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/usb/bos.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

USBD_DEVICE_DEFINE(my_usbd, DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0)), 0x1209, 0x0001);
USBD_DEVICE_QUALIFIER_DEFINE(my_usbd_dev_qualifier);
USBD_CONFIGURATION_DEFINE(my_usbd_config, USB_SCD_SELF_POWERED, 100);
USBD_BOS_DEFINE(my_usbd_bos);
USBD_DESC_LANG_DEFINE(my_usbd_lang);
USBD_DESC_MANUFACTURER_DEFINE(my_usbd_manufacturer, "tinyVision.ai");
USBD_DESC_PRODUCT_DEFINE(my_usbd_product, "tinyCLUNX33");
//USBD_DESC_SERIAL_NUMBER_DEFINE(my_usbd_serial_number, "0123456789ABCDEF");

int main(void)
{
	const struct device *udc0 = DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0));
	int err;

	err = usbd_add_descriptor(&my_usbd, &my_usbd_dev_qualifier);
	err |= usbd_add_descriptor(&my_usbd, &my_usbd_bos);
	err |= usbd_add_descriptor(&my_usbd, &my_usbd_lang);
	err |= usbd_add_descriptor(&my_usbd, &my_usbd_manufacturer);
	err |= usbd_add_descriptor(&my_usbd, &my_usbd_product);
	//err |= usbd_add_descriptor(&my_usbd, &my_usbd_serial_number);
	err |= usbd_add_configuration(&my_usbd, &my_usbd_config);
	err |= usbd_register_class(&my_usbd, "uvc_0", 1);
	__ASSERT_NO_MSG(err == 0);

	err = usbd_init(&my_usbd);
	__ASSERT_NO_MSG(err == 0);

	err = usbd_enable(&my_usbd);
	__ASSERT_NO_MSG(err == 0);

	while (true) {
		usb23_irq_handler(udc0);
		k_sleep(K_MSEC(10));
	}

	return 0;
}