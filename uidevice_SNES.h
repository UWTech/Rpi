/* by Eamon Maguire 5/13/2013 (maguire_Eamon@yahoo.com)**please leave this intact
  makes a uidevice (virtual keyboard or mouse) to send keyboard input to the kernel. For use in custom controller drivers.
	NUM_KEYS is the size of the keys array, and NUM_EVENTS is the size of the events array.
	For complete details about uinput and input (e.g.: keys, event types) , see the source code found at the links next to the #include statements
	this software is released under the GNU free license agreement:http://www.gnu.org/licenses/fdl.html

*/

#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<unistd.h>
#include<cstring>// for memset
#include<linux/input.h>//for creating keyboard events. header file: http://lxr.free-electrons.com/source/include/linux/input.h
#include<linux/uinput.h>//for creating devices to generate keyboard events. uibit details: http://lxr.free-electrons.com/ident?i=UI_SET_EVBIT
#include<sys/ioctl.h>//for ioctl

#define NUM_KEYS 4//defines the number of keys to add to the device
#define NUM_EVENTS 2//defines the number of event types to register with the uinput device

#ifndef UIDEVICE_SNES_H
#define UIDEVICE_SNES_H

//arrays containing data (terms defined in Linux/input.h and Linux/uinput.h)
static int keys [ NUM_KEYS ] = { KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT }; //defines key codes to register with device
static int events [ NUM_EVENTS ] = { EV_KEY, EV_SYN };//defines events to be registered with the uidevice

using namespace std;	
class uiDevice_SNES
{
	public:
	uiDevice_SNES();
	int make_uiDevice(void);//creates a uidevice and returns the associated file descriptor
	bool unmake_uiDevice(void);//destroys the uidevice associated with fd

	private:
	int fd;//holds the file descriptor
	int ret;//holds ioctl return value


	//helper methods
	bool Register_Event(void);//registers the events specified by the events array
	bool Register_Keys(void);//registers the keys specified by the keys array

};
#endif
