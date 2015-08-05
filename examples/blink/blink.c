// Dash Blink Example
//
// Cycle through blinking the red, green, blue LEDs on the dash every second.
// To access the LEDs the following GPIOs are used:
//   - Red   = PB6
//   - Green = PB7
//   - Blue  = PA8
// When the GPIO is pulled down to a low level the LED will turn on.
//
// Copyright (c) 2015 Tony DiCola
// Released under a MIT license: http://opensource.org/licenses/MIT
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>


// Global state:
volatile uint32_t systick_millis = 0;  // Millisecond counter.


// Delay for the specified number of milliseconds.
// This is implemented by configuring the systick timer to increment a count
// every millisecond and then busy waiting in a loop.
static void delay(uint32_t milliseconds) {
  uint32_t target = systick_millis + milliseconds;
  while (target > systick_millis);
}

// Setup the systick timer to increment a count every millisecond.  This is
// useful for implementing a delay function based on wall clock time.
static void systick_setup(void) {
  // The Dash has a 26mhz external crystal and the CPU will run at that speed
  // if no other clock changes are applied.  To make the systick timer reset
  // every millisecond (or 1000 times a second) set its reload value to:
  //   CPU_CLOCK_HZ / 1000
  systick_set_reload(26000);
  // Set the systick clock source to the main CPU clock and enable it and its
  // reload interrupt.
  systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
  systick_counter_enable();
  systick_interrupt_enable();
}

// Systick timer reload interrupt handler.  Called every time the systick timer
// reaches its reload value.
void sys_tick_handler(void) {
  // Increment the global millisecond count.
  systick_millis++;
}

// Setup and configure the GPIOs to control the LEDs on the Dash.
static void gpio_setup(void) {
  // Enable the GPIO clocks for the two GPIO ports that will be used (A & B).
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);

  // Set each LED GPIO as an output.
  gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8); // PA8, blue LED
  gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO6); // PB6, red LED
  gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO7); // PB7, green LED
}


int main(void) {
  // Setup systick timer and GPIOs.
  systick_setup();
  gpio_setup();

  // Main loop.
  while (true) {
    // Light the red LED.  Note that LEDs light up when the GPIO is pulled low
    // with the gpio_clear function, and turn off when pulled high with the
    // gpio_set function.
    gpio_clear(GPIOB, GPIO6);  // Red LED on
    gpio_set(GPIOB, GPIO7);    // Green LED off
    gpio_set(GPIOA, GPIO8);    // Blue LED off
    delay(1000);  // Wait 1 second (1000 milliseconds).
    // Now light just the green LED.
    gpio_set(GPIOB, GPIO6);    // Red LED off
    gpio_clear(GPIOB, GPIO7);  // Green LED on
    gpio_set(GPIOA, GPIO8);    // Blue LED off
    delay(1000);
    // Finally light just the blue LED.
    gpio_set(GPIOB, GPIO6);    // Red LED off
    gpio_set(GPIOB, GPIO7);    // Green LED off
    gpio_clear(GPIOA, GPIO8);  // Blue LED on
    delay(1000);
  }

  return 0;
}
