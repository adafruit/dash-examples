/*
 * Dash Linker Configuration
 * Author: Tony DiCola
 *
 * This file is based on the libopencm3-examples project which is released with
 * the following license:
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Define memory regions for Dash's STM32F205GR6 CPU, with 1 megabyte of flash
 * and 128 kilobytes of memory.
 */
MEMORY
{
	rom (rx) : ORIGIN = 0x08000000, LENGTH = 1024K
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

/* Include the common ld script. */
INCLUDE libopencm3_stm32f2.ld
