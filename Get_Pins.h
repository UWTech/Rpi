/* Eamon Maguire 5/23/2013 (maguire_eamon@yahoo.com) **please leave intact
  Gets the current level (high or low, 0 or 1) of the GPIO pins associated with one of the two 32-bit pin level registers
	register found at offset GPIO_BASE + 13 = pins 0-31, GPIO_BASE + 14 = pins 32-53
	this software is released under the GNU free license agreement:http://www.gnu.org/licenses/fdl.html
*/

#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/mman.h>


//memory map setup courtesy of Linux open source:http://elinux.org/RPi_Low-level_peripherals#GPIO_Driving_Example_.28C.29
#define BCM2708_PERI_BASE	0x20000000
#define GPIO_BASE	(BCM2708_PERI_BASE + 0x200000) //location of GPIO controller

//define page and block size
#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

#ifndef GET_PINS_H
#define GET_PINS_H

using namespace std;
class Get_Pins
{
	public:

	Get_Pins();//default register setting is 13 offsets from the base: pins 0-31
	Get_Pins(int pin_num);//takes the pin number as an argument, and selects the appropriate register ( 13 offsets for 0-31, 14 offsets for 32-53 )

	int getPins(void);//returns the value of the specified register

	private:
	
	int register_select;//number of 32-bit offsets from the first GPIO register
	volatile unsigned *gpio;//used to point to the memory map. Holds a pointer to the GPLEV register after constructor has assigned memory map via setUpIO
	volatile unsigned * setupIO(void);//returns a pointer to the memory map used to set up io pins
};
#endif
