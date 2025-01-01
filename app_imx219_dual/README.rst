# USB IMX219 Example

Tested with:
[rtl011](https://github.com/tinyvision-ai-inc/tinyclunx33_public/releases/tag/rtl011)


## Building guide

Visit the
[`tinyclunx33_sdk`](https://github.com/tinyvision-ai-inc/zephyr_internal/tree/tinyclunx33_sdk)
documentation for a complete guide.

Example using the Rev2 SoM, Rev2 Devkit, FPGA releasea `rtl011`:

```
west build --board tinyclunx33@rev2/rtl011 --shield tinyclunx33_devkit_rev2
west flash
```


## Crafting guide

This uses the USB Video class (UVC) to display the image feed from an IMX219 camera over MIPI.
Two video outputs streams are given for a single video input.

- Attach an [IMX219 module](https://tinyvision.ai/products/imx219-raspberry-pi-camera-v2) to an FPC cable

- Attach a
  [Syzygy dual](https://tinyvision.ai/products/syzygy-adapters) or
  [Syzygy in/out](https://tinyvision.ai/products/syzygy-mipi-to-usb-input-output-adapter)
  adapter to the other end.

- Plug the adapter to the adapter on the right side: `Expansion #2` for Rev1, `Pod A` for Rev2


## Running guide

After programming the device and power cycling the board, a video interface
would show-up, such as `/dev/video0` or `/dev/video2` on Linux.

See the
[UVC](https://github.com/tinyvision-ai-inc/zephyr/blob/pr-usb-uvc/samples/subsys/usb/uvc/README.rst#playing-the-stream)
Zephyr sample for a more commplete guide of using UVC.
