/* by Eamon Maguire (Maguire_Eamon@yahoo.com) 5/12/2013
  class sets GPIO pin values as specified by user. 
	Default constructor create a pin mask and offset which set GPIO pins 22-25 input (leaving the other pins in the register as they were prior to set)
	Assumes BCM pin number schema
	Pin Mask is used to bitwise-and the contents of the specified register. 
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

#ifndef SET_PINS_H
#define SET_PINS_H

using namespace std;
class Set_Pins
{
	public:

	Set_Pins();
	Set_Pins(int pin_mask, int register_offset);

	bool setPins(void);//sets pins up for the controller

	private:
	
	int register_select;//number of 32-bit offsets from the first GPIO register
	int pin_mask;// mask for setting and clearing bits in selected regiser
	
	volatile unsigned * setUpIO(void);//returns a pointer to the memory map used to set up io pins
};
#endif
