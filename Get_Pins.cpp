/* Eamon Maguire 5/23/2013 (maguire_eamon@yahoo.com) **please leave this intact
  definitions for the methods of the Get_Pins class
	Gets the current level ( high or low, 0 or 1 ) of the GPIO pins associated with one of the two pin level registers
	register found at offset GPIO_BASE + 13 = pins 0-31, GPIO_BASE + 14 = pins 32-53
	this software is released under the GNU free license agreement:http://www.gnu.org/licenses/fdl.html
*/

#include"Get_Pins.h"

using namespace std;

//memory map setup courtesy of Linux open source:http://elinux.org/RPi_Low-level_peripherals#GPIO_Driving_Example_.28C.29
// Gert van Loo and Dom
// Set up a memory regions to access GPIO
//
volatile unsigned * Get_Pins::setupIO()
{
	int mem_fd;//for storing file descriptor associated with memory map
	void *gpio_map;//pointer to memory map

   /* open /dev/mem */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("can't open /dev/mem \n");
      return NULL;
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
      return NULL;
   }

   // Always use volatile pointer! 
   //return the pointer to the memory region for purposes of accessing the GPIO register
   return (volatile unsigned *)gpio_map;


} ;// setup_io

//default constructor sets register select to GPLEV0 (pins 0-31)
Get_Pins::Get_Pins()
{
	register_select = 13;//define GPLEV0 to be returned

		// I/O access
	gpio = setupIO();//get a pointer to the memory map
	
	if(gpio == NULL)//check for null pointer
		exit(EXIT_FAILURE);
};

//pins 0-31 are 13 offsets from the base controller, and pins 32-53 are 14 offsets from the base controller
//assumes BCM processor view number schema
Get_Pins::Get_Pins(int nPin_Number)
{
	if( nPin_Number > 53 || nPin_Number < 0 )
	{
		cerr << " error: no such GPIO pin. Pin level value not returned. Pin number = " << nPin_Number << "\n";
		exit(EXIT_FAILURE);
	}
	else if( nPin_Number <= 31)//pins 0-31 are in GPLEV0
	{
		register_select = 13;//13 32-bit offsets from the controller
	}
	else//pins 32 - 53 are in GPLEV1
		register_select = 14;//14 32-bit offsets from the controller

	// I/O access
	gpio = setupIO();//get a pointer to the memory map
	
	if(gpio == NULL)//check for null pointer
		exit(EXIT_FAILURE);
};


//returns the pin level values found in the specified pin level register
int Get_Pins::getPins()
{
	//get the pin levels from the pin level register
	return *( gpio + register_select );//return the pin levels
};
