// Dash Button Example
//
// Demonstrates how to read the button on the dash by turning on the blue LED
// when the button is pressed.  One side of the button is connected to PA0 and
// the other side is connected through a pull down to ground.  By enabling PA0
// as an input with a pullup resistor you can detect when the button is pressed.
//
// Copyright (c) 2015 Tony DiCola
// Released under a MIT license: http://opensource.org/licenses/MIT
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>


// Setup and configure the GPIOs to read the button and control the blue LED.
static void gpio_setup(void) {
  // Enable the GPIO clock for port A.
  rcc_periph_clock_enable(RCC_GPIOA);

  // Set the blue LED GPIO (PA8) as an output.
  gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8);

  // Set the button GPIO (PA0) as an input with an internal pull-up resistor.
  gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO0);
}


int main(void) {
  // Setup GPIOs.
  gpio_setup();

  // Main loop.
  while (true) {
    // Check if the button is pressed by reading PA0 to see if it's at a high level.
    if (gpio_get(GPIOA, GPIO0) > 0) {
      // Button is pressed, light up the blue LED (pull its GPIO low to light).
      gpio_clear(GPIOA, GPIO8);
    }
    else {
      // Button is not pressed, turn off the blue LED by pulling its GPIO high.
      gpio_set(GPIOA, GPIO8);
    }
  }

  return 0;
}
