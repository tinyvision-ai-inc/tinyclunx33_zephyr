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
west init -m https://github.com/josuah/tinyclunx33_zephyr_example
west update
west build -p -b litex_vexriscv tinyclunx33_zephyr_example
```

As a result, `build/zephyr/zephyr.bin` should be created, ready  to be loaded
into the SoM flash as described below.


## Loading the images to the flash

First, connect both USB cables onto the tinyCLUNX33 devboard featuring  a
tinyCLUNX SoM.
One is hooked to the FTDI for debugging, one is hooked to the CrossLink-U NX
USB interface.

Then, as documented on the
[tinyCLUNX33 SoM flash](https://tinyclunx33.tinyvision.ai/md_som_flash.html)
documentation, it is possible to load the FPGA and Zephyr binaries to the flash
at the expected offset:

```
ecpprog -o 0x00000000 tinyclunx33_zephyr_example/bin/fpga_top_som_impl.devcon23.bit
ecpprog -o 0x00100000 build/zephyr/zephyr.bin
```

Once both binaries are loaded, it is necessary to unplug the FTDI Debug USB
interface and replug it, which will power-cycle the board, and let the FPGA
start.

Then, an USB device should appear, recognized as a USB serial console, with
an access to the Zephyr shell.
