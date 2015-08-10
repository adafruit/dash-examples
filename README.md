# Dash Example Code

Example code to run on an Amazon Dash using the [libopencm3 library](http://libopencm3.org/wiki/Main_Page).
Note that right now the examples only show how to run code that uses the Dash's
STM32F205 CPU and **not** the Broadcom WiFi module.  This means you won't be able
to use the WiFi radio, but can still change the LEDs, read the button, access a
UART and more.  Usage of WiFi will require the [WICED SDK](http://community.broadcom.com/community/wiced-wifi)
and a little more investigation of how the CPU is connected to the WiFi module.

The following examples are available:

*   Blink - Blink each red, green, blue LED individually for a second.
*   PWM - Use PWM to set the red, green, blue LEDs to any RGB color.
*   UART - Use the UART exposed on PC6 & PC7 to send out data from the Dash.

## Requirements

To use this example code you must have a GCC ARM toolchain setup and in your
path.  I recommend using [this Vagrant-based GCC ARM toolchain virtual machine](https://github.com/adafruit/ARM-toolchain-vagrant)
to easily get a toolchain that can work from any OS.

You will also need a STLink V2 programmer connected to at least the Dash's SWDCLK,
SWDIO, and RESET pads.  The examples are made to be easily uploaded to a Dash
using the [stlink Linux command line tools](https://github.com/texane/stlink)
(installed automatically in the mentioned GCC ARM virtual machine).

## Usage

This repository should be cloned from git with the `--recursive` option so that
the libopencm3 submodule is cloned.  For example run (inside the GCC ARM virtual
machine if using it):

    git clone --recursive https://github.com/adafruit/dash-examples.git

However if you accidentally don't use the recursive option when cloning you can
run these commands from inside the repository to fix it:

    git submodule init
    git submodule update

Once the submodule is downloaded you **must** compile the libopencm3 library first
by running this command from the root:

    make

After the library compiles you can navigate to any of the example directories
and compile & run them also using make.  For example to compile the .hex output
of the blink example run:

    cd examples/blink
    make hex

If you receive an error that no target is found for 'blink.elf' or similar then go
back and make sure you compiled the libopencm3 library in the repository root first.

Inside an example directory you can also run `make clean` to clear all the binaries
that were generated.

To flash an example to the board run the `make stlink-flash` command.  This will
use the st-flash utility to send the example binary to the Dash using a STLink V2
programmer.  You should see output like the following on success:

    2015-08-05T21:02:49 INFO src/stlink-common.c: Loading device parameters....
    2015-08-05T21:02:49 INFO src/stlink-common.c: Device connected is: F2 device, id 0x20036411
    2015-08-05T21:02:49 INFO src/stlink-common.c: SRAM size: 0x20000 bytes (128 KiB), Flash: 0x100000 bytes (1024 KiB) in pages of 131072 bytes
    2015-08-05T21:02:49 INFO src/stlink-common.c: Attempting to write 916 (0x394) bytes to stm32 address: 134217728 (0x8000000)
    EraseFlash - Sector:0x0 Size:0x4000
    Flash page at addr: 0x08000000 erased
    2015-08-05T21:02:49 INFO src/stlink-common.c: Finished erasing 1 pages of 16384 (0x4000) bytes
    2015-08-05T21:02:49 INFO src/stlink-common.c: Starting Flash write for F2/F4
    2015-08-05T21:02:49 INFO src/stlink-common.c: Successfully loaded flash loader in sram
    size: 916
    2015-08-05T21:02:49 INFO src/stlink-common.c: Starting verification of write complete
    2015-08-05T21:02:49 INFO src/stlink-common.c: Flash written and verified! jolly good!

If you receive an error that a STLink can't be found, make sure the STLink is
connected to the Dash, your computer, and if running in a virtual machine the
STLink is passed through to the VM (should automatically happen if using the
mentioned GCC ARM VM).

## Reviving A Bricked Dash

If you try to program the Dash's CPU and receive errors that the STM32 processor
cannot be detected (and you know for sure it's not a physical connection or software
issue) you can follow the steps mentioned [in the workaround section of this page](http://nuttx.org/doku.php?id=wiki:howtos:jtag-debugging)
to get the Dash back into a good state.  

If using the GCC ARM VM perform the following steps.  First connect the RESET
pin of the Dash to ground.  This will force the chip into a 'safe mode' and will
stop execution of your faulty program.  Then run OpenOCD's telnet server (which
is already installed in the VM) with the following command:

    openocd -f interface/stlink-v2.cfg -f target/stm32f2x.cfg

Note that this command will not return and OpenOCD will wait for a connection.
Now open a new terminal and connect to the VM again (using `vagrant ssh`) and run:

    telnet localhost 4444

Once connected to OpenOCD's telnet server run the following command:

    reset halt

This command should fail with a timed out error, like:

    timed out while waiting for target halted
    TARGET: stm32f2x.cpu - Not halted
    in procedure 'reset'
    in procedure 'ocd_bouncer'

Now remove the connection between RESET and ground.  You should see OpenOCD
report that the CPU is halted:

    target state: halted
    target halted due to debug-request, current mode: Thread
    xPSR: 0x01000000 pc: 0x080002cc msp: 0x20020000

Now run the following command to erase the flash memory and any faulty program:

    stm32f2x mass_erase 0

After a few seconds the erase should complete:

    device id = 0x20036411
    flash size = 1024kbytes
    stm32x mass erase complete

Now stop OpenOCD and close the telnet session by running the `shutdown` command.
You should now be able to flash an example program back to the chip (like the blink
example).  Be careful not to flash the same program that caused the problem or
else you might need to un-brick the device again!
