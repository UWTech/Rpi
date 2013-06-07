/* by Eamon Maguire (Maguire_Eamon@yahoo.com) 5/12/2013
  Definition for methods that set GPIO pins according to user specified arguments. 
	Default constructor create a pin mask and offset which set GPIO pins 22-25 input (leaving the other pins in the register as they were prior to set)
	Assumes BCM pin number schema:
	pins 0-9 at offset 0
	pins 10-19 at offset 1
	pins 20-29 at offest 2
	... etc.
	offset is in terms of distance from the base GPIO controller address defined in Set_Pins.h
	Each offset (1, 2, 3, etc.) represents one 32-bit register offset from the base address
	Pin Mask is used to bitwise-and the contents of the specified register. 
	this software is released under the GNU free license agreement:http://www.gnu.org/licenses/fdl.html
*/

#include"Set_Pins.h"

using namespace std;

//default constructor sets pins 22-25 to input
Set_Pins::Set_Pins()
{
	pin_mask = 0xFFFC003F;//define which of the pins in the register specified by REGISTER to set (in this case four pins set to input, bits 6-17 cleared, BCM pins 22-25 are input)
	register_select = 2;//define register to manipulated (0 = first register of the GPIO select, 1 = second, etc.);
	
};

//mask and 32-bit offsets from first register (10 pins in each register, occupying 3 bits each. the high order 2 bits are unused.)
Set_Pins::Set_Pins(int mask, int offset)
{
	if(offset > 5 || offset < 0)
	{
		cerr << "GPIO register offset invalid, register offset must be between 0 and 5 \n";
		exit(EXIT_FAILURE);
	}

	pin_mask = mask;
	register_select = offset;
};


//memory map setup courtesy of Linux open source:http://elinux.org/RPi_Low-level_peripherals#GPIO_Driving_Example_.28C.29
// Gert van Loo and Dom
// Set up a memory regions to access GPIO
//
volatile unsigned * setupIO()
{
	int mem_fd;//for storing file descriptor associated with memory map
	void *gpio_map;//pointer to memory map

   /* open /dev/mem */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("can't open /dev/mem \n");
      exit(-1);
   }

   /* mmap GPIO */
   gpio_map = mmap(
      NULL,             //Any adddress in our space will do
      BLOCK_SIZE,       //Map length
      PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
      MAP_SHARED,       //Shared with other processes
      mem_fd,           //File to map
      GPIO_BASE         //Offset to GPIO peripheral
   );

   close(mem_fd); //No need to keep mem_fd open after mmap

   if (gpio_map == MAP_FAILED) {
      printf("mmap error %d\n", (int)gpio_map);//errno also set!
      exit(-1);
   }

   // Always use volatile pointer! 
   //return the pointer to the memory region for purposes of accessing the GPIO register
   return (volatile unsigned *)gpio_map;


} ;// setup_io

bool Set_Pins::setPins()
{

	// I/O access
	volatile unsigned *gpio = setupIO();//get a pointer to the memory map
	
	if(gpio == NULL)//check for null pointer
		return false;

	//clear or set the bits associated with the selected register using the pin mask.
	*( gpio + register_select ) &= pin_mask;

	gpio = NULL;

	return true;
};
