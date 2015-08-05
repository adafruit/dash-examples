// Dash LED PWM Example
//
// Use timers to control the LEDs with PWM and light them to any desired color.
// Will loop through all possible hues of color on the LEDs.
//
// Copyright (c) 2015 Tony DiCola
// Released under a MIT license: http://opensource.org/licenses/MIT
#include <math.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>


// Configuration:
#define PWM_FREQ 500  // Desired update rate of the LED PWM signal.

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
  // By default the Dash CPU will use an internal 16mhz oscillator for the CPU
  // clock speed.  To make the systick timer reset every millisecond (or 1000
  // times a second) set its reload value to:
  //   CPU_CLOCK_HZ / 1000
  systick_set_reload(16000);
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

// Setup and configure the timers that will generate a PWM signal to dim the LEDs.
static void pwm_setup(void) {
  // Enable the GPIO and timer clocks that will be used.
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_TIM1);
  rcc_periph_clock_enable(RCC_TIM4);

  // Now setup the LED GPIOs to show their associated timer PWM channel output.
  // The mapping of LED GPIOs to timer channels is (from Table 10 of the datasheet):
  //  - PA8 (blue LED)  = Timer 1, channel 1
  //  - PB6 (red LED)   = Timer 4, channel 1
  //  - PB7 (green LED) = Timer 4, channel 2

  // Setup each LED to use its alternative function (GPIO_MODE_AF).
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO8); // PA8, blue LED
  gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6); // PB6, red LED
  gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO7); // PB7, green LED

  // Configure the alternate function for each LED to use the timer PWM output.
  // See Table 10 in the datasheet for full matrix of pins and their alternate
  // function values.
  gpio_set_af(GPIOA, GPIO_AF1, GPIO8);
  gpio_set_af(GPIOB, GPIO_AF2, GPIO6);
  gpio_set_af(GPIOB, GPIO_AF2, GPIO7);

  // Configure both timers for PWM mode.  In PWM mode the timers will count up
  // for a specified period and the output of each timer channel will be set
  // high or low depending on if the timer value is above or below a threshold.

  // First configure timer 1.
  // Reset the timer configuration and then set it up to use the CPU clock,
  // center-aligned PWM, and an increasing rate.
  timer_reset(TIM1);
  timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_CENTER_1, TIM_CR1_DIR_UP);
  // Divide counter by 16 to scale it down from 16mhz clock speed to a 1mhz rate.
  timer_set_prescaler(TIM1, 16);
  // Set timer period by solving:
  //   PWM frequency = timer clock speed / timer period
  timer_set_period(TIM1, 1000000/PWM_FREQ);
  timer_enable_break_main_output(TIM1);  // Must be called for advanced timers
                                         // like this one.  Unclear what this
                                         // does or why it's necessary but the
                                         // libopencm3 timer and STM32 docs
                                         // mention it.

  // Apply the exact same configuration to timer 4.
  timer_reset(TIM4);
  timer_set_mode(TIM4, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_CENTER_1, TIM_CR1_DIR_UP);
  timer_set_prescaler(TIM4, 16);
  timer_set_period(TIM4, 1000000/PWM_FREQ);
  timer_enable_break_main_output(TIM4);

  // Now setup each timer channel that is connected to a LED.  Each channel can
  // have a different PWM threshold set so that it can be uniquely controlled
  // (the rest of the timer configuration like period, etc. is shared by all
  // channels on a timer).

  // PA8 Blue LED is timer 1, channel 1.
  // Enable the channel PWM output.
  timer_enable_oc_output(TIM1, TIM_OC1);
  // Set the PWM mode to 2 which means the PWM signal is low (LED turns on) when
  // the timer value is below the threshold and high (LED off) above it.
  timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_PWM2);

  // PB6 Red LED is timer 4, channel 1.
  timer_enable_oc_output(TIM4, TIM_OC1);
  timer_set_oc_mode(TIM4, TIM_OC1, TIM_OCM_PWM2);

  // PB7 Green LED is timer 4, channel 2.
  timer_enable_oc_output(TIM4, TIM_OC2);
  timer_set_oc_mode(TIM4, TIM_OC2, TIM_OCM_PWM2);

  // Turn on both timers.
  timer_enable_counter(TIM1);
  timer_enable_counter(TIM4);

  // Set the threshold for each channel to zero so they are off.  Setting the
  // threshold to any value between 0 and 2000 will set the LED intensity (with
  // 0 = off and 2000 = fully lit).
  timer_set_oc_value(TIM1, TIM_OC1, 0);  // Blue LED
  timer_set_oc_value(TIM4, TIM_OC1, 0);  // Red LED
  timer_set_oc_value(TIM4, TIM_OC2, 0);  // Green LED
}

// Set the color of the LED to the provided red, green, and blue intensity
// value.  Each component should be a value from 0 to 1.0 where 0 is off and
// 1.0 is full intensity.  For example a cyan (full green and blue) color would
// be led_color(0, 1, 1).
static void led_color(float red, float green, float blue) {
  // Scale each floating point value to be within the range of possible timer
  // periods (0 to 2000 if running at 500hz), then set the timer channel
  // threshold to that value.
  timer_set_oc_value(TIM1, TIM_OC1, blue*(1000000/PWM_FREQ));
  timer_set_oc_value(TIM4, TIM_OC1, red*(1000000/PWM_FREQ));
  timer_set_oc_value(TIM4, TIM_OC2, green*(1000000/PWM_FREQ));
}

// Convert from HSV to RGB color formats.  HSV is nice for pretty hue animations.
// Hue (h) is a float from 0 to 360 degrees, saturation (s) is a float from 0 to
// 1 (full saturation), and value (v) is a float from 0 to 1 (full value).
// Based on code from:
//  http://www.cs.rit.edu/~ncs/color/t_convert.html
static void hsv_to_rgb(float h, float s, float v, float* r, float* g, float* b) {
  if (s == 0) {
    // achromatic (grey)
    *r = *g = *b = v;
    return;
  }
  h /= 60;      // sector 0 to 5
  int i = floor(h);
  float f = h - i;      // factorial part of h
  float p = v * (1 - s);
  float q = v * (1 - s * f);
  float t = v * (1 - s * (1 - f));
  switch (i) {
    case 0:
      *r = v;
      *g = t;
      *b = p;
      break;
    case 1:
      *r = q;
      *g = v;
      *b = p;
      break;
    case 2:
      *r = p;
      *g = v;
      *b = t;
      break;
    case 3:
      *r = p;
      *g = q;
      *b = v;
      break;
    case 4:
      *r = t;
      *g = p;
      *b = v;
      break;
    default:    // case 5:
      *r = v;
      *g = p;
      *b = q;
      break;
  }
}

int main(void) {
  // Setup systick timer and PWM channels.
  systick_setup();
  pwm_setup();

  float hue = 0.0;

  // Main loop.
  while (true) {
    // Convert the current hue to a RGB color.
    float r, g, b;
    hsv_to_rgb(hue, 1.0, 1.0, &r, &g, &b);
    // Set the LED to the specified color.
    led_color(r, g, b);
    // Now increment the hue, wrapping back to zero when necessary, and delay
    // for a bit until the next loop.
    hue += 1.0;
    if (hue >= 360.0) {
      hue = 0.0;
    }
    delay(10);
  }

  return 0;
}
