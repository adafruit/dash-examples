// Dash UART Example
//
// Send debug output on a UART connected to UART6 (PC6 & PC7).  Right now only
// sending data out the TX pin (PC6) is demonstrated.  You will want to connect
// a serial to USB cable's RX pin to the Dash's TX pin (PC6).  Then open the
// serial terminal at 115200 baud with a standard 8n1 confige (8 data bits, 1
// stop bit, no parity).
//
// NOTE: The Dash's UART is NOT 5V safe!  Don't connect the Dash RX (PC7) to a
// 5 volt serial to USB cable TX!  You can however safely use the Dash TX with
// a 5 volt cable's RX pin.
//
// Copyright (c) 2015 Tony DiCola
// Released under a MIT license: http://opensource.org/licenses/MIT
#include <errno.h>
#include <stdio.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>


// Declare newlib-nano's _write function to support printf.  Definition is
// further below.
int _write(int fd, char *ptr, int len);

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

// Setup the clocks and GPIO pins to enable the UART.
static void uart_setup(void) {
  // Turn on the UART and its associated GPIO pin clock.
  rcc_periph_clock_enable(RCC_USART6);
  rcc_periph_clock_enable(RCC_GPIOC);

  // Set PC6 & PC7 to their alternate function 8, UART6.  PC6 will be UART TX
  // (out of Dash), and PC7 will be UART RX (into Dash).  See Table 10 of the
  // datasheet for the table of all pins and possible functions.
  gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6 | GPIO7);
  gpio_set_af(GPIOC, GPIO_AF8, GPIO6 | GPIO7);

  // Setup the UART parameters.
  usart_set_baudrate(USART6, 115200);
  usart_set_databits(USART6, 8);
  usart_set_stopbits(USART6, USART_STOPBITS_1);
  usart_set_parity(USART6, USART_PARITY_NONE);
  usart_set_flow_control(USART6, USART_FLOWCONTROL_NONE);
  usart_set_mode(USART6, USART_MODE_TX_RX);

  // Enable the UART
  usart_enable(USART6);
}

// _write function will be called by newlib-nano's printf and other standard
// output functions.  This will just send all the received data out the UART.
int _write(int fd, char* ptr, int len) {
  // Fail if trying to write to anything besides standard output (FD = 1).
  if (fd != 1) {
    errno = EBADF;
    return -1;
  }
  // Send the data out the UART a character at a time.
  int i;
  for (i = 0; i < len; i++)
    usart_send_blocking(USART6, ptr[i]);
  return i;
}


int main(void) {
  // Setup systick timer and UART.
  systick_setup();
  uart_setup();

  printf("Hello world!\r\n");

  int count = 0;

  // Main loop.
  while (true) {
    // Print out the current count and increment it.  Wait for a second and repeat.
    printf("Count: %d\r\n", count++);
    delay(1000);
  }

  return 0;
}
