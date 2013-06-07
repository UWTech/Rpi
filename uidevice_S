/* by Eamon Maguire 5/13/2013 (maguire_Eamon@yahoo.com) **please leave this intact
  Contains definitions of the uiDevice_SNES class.
	makes a uidevice (virtual keyboard or mouse) to send keyboard input to the kernel. For use in custom controller drivers.
	To add or remove allowable keystrokes, simply modify the array found in uiDevice_SNES.h and adjust the size variables accordingly
this software is released under the GNU free license agreement:http://www.gnu.org/licenses/fdl.html
*/

#include"uiDevice_SNES.h"

using namespace std;

//defualt constructor
uiDevice_SNES::uiDevice_SNES()
{
	fd = 0;
	ret = 0;
};


//creates a ui device capable of all keys and events specified in the keys and events arry found ing uiDevice_SNES.h
//returns the file descriptor number associated with the uidevice upon success, or -1 on failure.
int uiDevice_SNES::make_uiDevice()
{

	//attempt to open a uinput in write only and non-blocking mode
	fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK); 

	//test for error conditions
	if(fd < 0)
	{
		cerr << " failed to open uinput, fd = " << fd;
		perror("Error ");
		return -1;
	}

	//register events and keys specified in array found in header file
	if( !(Register_Event()) ) return -1;
	if( !(Register_Keys()) )  return -1;

	//instantiate the uinput struct
	struct uinput_user_dev uiDevice;
	//allocate a block of memory for the device, and register a pointer
	memset(&uiDevice, 0, sizeof(uiDevice));
	
	//name the device and set other variables
	snprintf(uiDevice.name, UINPUT_MAX_NAME_SIZE, "uiDevice");
	uiDevice.id.bustype = BUS_USB;
	uiDevice.id.vendor  = 0x1337;//note, the vendor chosen is irrelevant. Any positive number will do
	uiDevice.id.product = 0x1337;//again, the product chosen is irrelevant. Any positive number will do
	uiDevice.id.version = 1;//also any number will do

	//write the structure to the associated file descriptor
	ret = write(fd, &uiDevice, sizeof(uiDevice));

	if(ret < 0)//test for error code
	{
		cerr << " write of ui struct to fd failure. ret = " << ret;
		perror("Error ");
		return -1;
	}

	//finally, attempt to create the device
	ret = ioctl(fd, UI_DEV_CREATE);
	
	if(ret < 0)//test for error code
	{
		cerr << " ui device creation failed. ret = " << ret;
		perror("Error ");
		return -1;
	}

	return fd;//return the file descriptor
};

//destroys the uiDevice
bool uiDevice_SNES::unmake_uiDevice(void)
{

	ret = ioctl(fd, UI_DEV_DESTROY);

	if(ret < 0)//test for error code
	{
		cerr << " ui device destruction failed. ret = " << ret;
		perror("Error ");
		return false;
	}

	return true;
};


//registers events associated with the events array found in the header file
bool uiDevice_SNES::Register_Event()
{

	//register all events held in the events array
	for( int i = 0; i < NUM_EVENTS; i++ )
	{
		ret = ioctl(fd, UI_SET_EVBIT, events[i]);//register event

		if(ret < 0)//test for error code
		{
			cerr << " event registration failed ioctl failed. event = " << events[i];
			perror("Error ");
			return false;
		}
	}

	return true;
};

//register keystrokes allowed by the uidevice
bool uiDevice_SNES::Register_Keys()
{
	int ret;//to hold return values

	for( int i = 0; i < NUM_KEYS; i++)
	{
		ret = ioctl(fd, UI_SET_KEYBIT, keys[i]);

		if(ret < 0)//test for error code
		{
			cerr << " keystroke ioctl failed. Keystroke = " << ret;
			perror("Error ");
			return false;
		}
	}


	return true;
};
