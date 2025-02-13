# Intro

My mini projects for stm32f103c8t6 (aka. bluepill).

# Compile

To install toolchain, see toolchain_build_guide.txt

And then:

`cd blink && make`

You need an **USB to TTL module** to flash.

Set BOOT0 to 1, press Reset button and then:

`make flash`

# Projects

**firmware.bin**: The original blink example in the board.

**blink**: A simple blink example.

**shell**: A shell example over serial port. (requires an USB to TTL module).

**cdcacm**: An ACM driver taken from `libopencm3-examples.git` that just echoes.
(Only requires an USB **data** cable to communicate, but you still need an USB to TTL module to flash)
