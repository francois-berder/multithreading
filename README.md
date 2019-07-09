# Cooperative multithreading

This shows how to create a small cooperative multithreading RTOS for ARM Cortex-M3/M4/M7.
This was tested on a nucleo-L452RE board and a nucleo-F722ZE board.

## Build and flash instructions

First, flash SEGGER firmware on your nucleo boards as described here: https://www.segger.com/products/debug-probes/j-link/models/other-j-links/st-link-on-board/.

Then, install J-Link Software and Documentation Pack.

To build, flash and debug firmware on your board:
```
$ BOARD=nucleo-l452re make                 # Build release by default
$ BOARD=nucleo-l452re CONFIG=debug make
$ BOARD=nucleo-f722ze make
$ BOARD=nucleo-l452re CONFIG=debug make
$ BOARD=nucleo-f722ze CONFIG=release make flash-target
$ BOARD=nucleo-l452re CONFIG=debug make debug-target
```
