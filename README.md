# tinyCLUNX33

Sample project showing how run Zephyr on a VexRiscv CPU running on the
tinyCLUNX33.

This project uses a binary generated by
[zephyr-on-litex-vexriscv](https://github.com/litex-hub/zephyr-on-litex-vexriscv)
modified to fit the needs of [tinyVision.ai](https://tinyvision.ai/).

A binary of the FPGA release is provided rather than the source as the RTL is
going to be restructured.

These instructions will be updated once the new RTL is available.


## Building zephyr.bin

```
# Prepare a workspace
mkdir "$HOME/zephyrproject"
cd "$HOME/zephyrproject"

# Initialize the repos in that workspace
west init -m https://github.com/tinyvision-ai-inc/tinyclunx33_zephyr_example
west update
```

Then depending on the demo to try, one of these:

```
west build -p -b litex_vexriscv tinyclunx33_zephyr_example/usb_uvc_example
west build -p -b litex_vexriscv tinyclunx33_zephyr_example/usb_cdc_example
```

As a result, `build/zephyr/zephyr.bin` should be created, ready to be loaded
into the SoM flash as described below.


## Loading the images to the flash

First, connect USB cables onto the tinyCLUNX33 devboard:
- One to the FTDI side (`DEBUG`)
- One to the CrossLinkU-NX side (`DATA`), optional

Then, as documented on the
[tinyCLUNX33 SoM flash](https://tinyclunx33.tinyvision.ai/md_som_flash.html)
documentation, it is possible to load the FPGA and Zephyr binaries to the flash
at their own offset:

```
ecpprog -s -o 0x00000000 tinyclunx33_rtl_reference_design_v0.5.bit
ecpprog -s -o 0x00100000 build/zephyr/zephyr.bin
```

Once both binaries are loaded, it is necessary to unplug the FTDI Debug USB
interface and replug it, which will power-cycle the board, and let the FPGA
start.

Then, an USB device should appear, recognized as a USB serial console, with
an access to the Zephyr shell.
See the [doc on Zephyr](https://tinyclunx33.tinyvision.ai/md_zephyr.html) for
how to access them.


## Running the `usb_cdc_example`

This uses the CDC ACM USB class, in a custom `usbd_cdc_raw.c` class to allow
to enqueue data without using a ring buffer.

After programming the device and power cycling the board, a serial interface
would show-up, such as on `/dev/ttyACM0` on Linux, and everything written to it
will be read back:

```
picocom /dev/ttyACM0
```

Debug logs are available from the FTDI UART interface, showing a hexdump
of every buffer read by the device, which it then submit back.

```
picocom -q -b 192000 /dev/ttyUSB1
```

## Running the `usb_uvc_example`

This uses the USB Video class (UVC), with the feed parameters for now hardcoded
into the `usbd_uvc.c` class implementation.

After programming the device and power cycling the board, a video interface
would show-up, such as on `/dev/video2` on Linux, and can be opened with tools
such as
[VLC](https://www.videolan.org/vlc/),
[Gstreamer](https://gstreamer.freedesktop.org/),
[ffplay](https://ffmpeg.org/ffplay.html),
[guvcview](https://guvcview.sourceforge.net/),
or libraries such as [OpenCV](https://opencv.org/),
or eventually [libuvc](https://github.com/libuvc/libuvc) for low-level access.

```
gst-launch-1.0 v4l2src device=/dev/video2 ! videoconvert ! autovideosink
mpv /dev/video2
ffplay /dev/video2
vlc v4l2:///dev/video2
```


## USB driver configuration:

Some elements of the driver allow manual configuration of some elements, which
might require tuning if writing your own application:

### `num-bidir-endpoints`

The number of USB endpoints, including the control endpoint (0x00 and 0x80),
with the IN and OUT direction counting together as 1.

So if your endpoints are: Control IN/OUT (0x00, 0x80), Bulk IN/OUT (0x01, 0x81)
then you have 2 bidir endpoints.

```
num-bidir-endpoints = <2>;
```

### `num-endpoint-trb`

The number of per-endpoint TRB buffers present in the system.
The TRB buffers are internal queue shared between Zephyr and the hardare.

The more buffers there are, the more backlog there can be on the USB bus,
which allows to enqueue a lot of transfers then work on something in the
meantime.

This is an array where each element is grouped by 2, for the OUT and IN
direction of each endpoint respectively.

The first if for the endpoint zero, and always required to be `1` and `1`

For instance, for endpoints 0x00, 0x80, 0x01, 0x81, 0x02, 0x82 respectively,
this would be:

```
num-endpoint-trb = <1 1  5 0  10 10>;
```
