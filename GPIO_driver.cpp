/* 5/08/2013 Eamon Maguire (maguire_eamon@yahoo.com)**please leave this intact
  Sets BCM pins 22,23,24,25 of the Raspberry Pi as input for purposes of driving a game controller.
	See BCM2835 datasheet for more details about GPIO registers: http://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
	See linux low level peripheral tutorial on liux site for more info about setting up a memory map on the Raspberry Pi:http://elinux.org/RPi_Low-level_peripherals#GPIO_Driving_Example_.28C.29
	this software is released under the GNU free license agreement:http://www.gnu.org/licenses/fdl.html

*/

#include<iostream>
#include<cstdlib>//for exit
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<unistd.h>
#include<linux/input.h>//for creating keyboard events:http://lxr.free-electrons.com/source/include/linux/input.h
#include<linux/uinput.h>//for creating devices to generate keyboard events:http://lxr.free-electrons.com/ident?i=UI_SET_EVBIT
#include<sys/ioctl.h>//for ioctl commands
#include<cstring>// for memset
#include<signal.h>//for signal catching
#include<time.h>//for nanosleep
#include"Set_Pins.h"//sets the GPIO pins of the specified register
#include"Get_Pins.h"//gets the pin level of the GPIO pins
#include"uiDevice_SNES.h"//creates the uidevice associated with the SNES driver

using namespace std;

void signal_handler(int signal);//forward reference for signal handler

//flag used to indicate if the ctrl-c has been pressed
static bool ctrl_c_pressed = false;

int main(int argc, char *argv[])
{
	int fd;//for storing file descriptor of the uinput device
	int ret;//holds value of ioctl return

	int nPins_Level;//holds the contents of the latest state of GPLEV register (high/low status of GPIO pins)
	int nTemp_GPLEV;//holds a copy of nPins_Level for purposes of using direction masks to isolate bits
	
	//pin masks to isolate the status of each direction's associated pin
	int nUp_Mask;
	int nDown_Mask;
	int nLeft_Mask;
	int nRight_Mask;

	//create an instance of the uidevice class
	uiDevice_SNES SNES;
	//create instance of set pins
	Set_Pins Pins;
	//create an instance of get pins
	Get_Pins Get;
	//create a timspec structure to hold a delay to use with nanosleep (prevents debouncing in the polling loop)
	struct timespec schmitt_delay;
	//create a signal action structure to handle the ctrl + c signal
	struct sigaction signal_structure;

	//define pin masks
	nUp_Mask = 0x02000000;//assumed to be BCM 25 (GPIO pin 25) if & GPLEV0 > 0, level is high = false 
	nDown_Mask = 0x01000000;//assumed to be BCM 24 (GPIO pin 24) ' '
	nLeft_Mask = 0x00800000;//assumed to be BCM 23 ' '
	nRight_Mask = 0x00400000;//assumed to be BCM 22

	//set the delay for the timespec structure to be used in nanosleep call
	schmitt_delay.tv_sec = 0;//set seconds delay to 0 (no seconds to delay)
	schmitt_delay.tv_nsec = 212000000L;//set nanoseconds delay 212M nanoseconds

	//create signal structure to handle the Linux exit command (ctrl + c = SIGINT)
	//http://hertaville.com/2012/11/18/introduction-to-accessing-the-raspberry-pis-gpio-in-c/
	signal_structure.sa_handler = signal_handler;//associate the signal handler with the structure
	signal_structure.sa_flags = 0;//unused flags
	sigemptyset(&signal_structure.sa_mask);

	//map the signal handler structure
	if (sigaction(SIGINT, &signal_structure, NULL) == -1) 
	{
        	cerr << "Problem with sigaction";
		perror(" sigaction handler error: ");
        	exit(EXIT_FAILURE);
    	}

	if( Pins.setPins() == false )//attempt to set up the GPIO pins as specified in the Set_Pins file (default constructor pins 22-25 input)
	{
		cerr << " Error: NULL pointer from memory map allocation";
		exit(EXIT_FAILURE);
	}


	fd = SNES.make_uiDevice();//attempt to create a uidevice and get the associated file descriptor (default = up, down, left, right, SYN, and KEY events.)
	
	if( fd < 0 )//test for error code
	{
		cerr << " ui device creation failed. ret = " << ret;
		perror("Error ");
		exit(EXIT_FAILURE);
	}

	
	//create a structure to write a keystroke
	struct input_event Atari_Key;

	//poll the pins to see if a direction on the joystick has been pressed
	// not pressed = high 1, pressed = low 0
	//NOTE: Because we are using a Linux kernel on the Raspberry Pi, user level interrupts triggered by GPIO pins are not possible.
	//The Linux kernel will not allow a user level process to cause an ISR to run baased on this input. Unfortunately, polling is the only solution to
	//driving the Atari controller.
	while(1)
	{
		nPins_Level = Get.getPins(); //retrieve the cuurent value of the GPLEV register

		nTemp_GPLEV = nPins_Level;//save the value to the temporary variable for purposes of using bitwise operations

		if( (nTemp_GPLEV &= nUp_Mask) == 0 )//test to see if up pin is low. (true)
		{
			cout << " up " << endl;
			
			memset(&Atari_Key, 0, sizeof(Atari_Key));//set aside a memory block and register a pointer

			Atari_Key.type = EV_KEY;//register event type, keystroke
			Atari_Key.code = KEY_UP;
			Atari_Key.value = 1;//press the key

			ret = write(fd, &Atari_Key, sizeof(Atari_Key));//write the event (keypress)

			if(ret < 0)//test for error code
			{
				cerr << " failed to create keyboard event. ret = " << ret;
				perror("Error keypress");
				exit(EXIT_FAILURE);
			}

			Atari_Key.type = EV_KEY;//register even type, keystroke 
			Atari_Key.code = KEY_UP;
			Atari_Key.value = 0; //release the key

			ret = write(fd, &Atari_Key, sizeof(Atari_Key));//write the event (key release)

			if(ret < 0)//test for error code
			{
				cerr << " failed to create keyboard event. ret = " << ret;
				perror("Error Key release");
				exit(EXIT_FAILURE);
			}


	            	Atari_Key.type = EV_SYN;//synchronize keyboard event
	            	Atari_Key.code = 0;
            		Atari_Key.value = 0;

			ret = write(fd, &Atari_Key, sizeof(Atari_Key));//write the event (key synchronize)

			if(ret < 0)//test for error code
			{
				cerr << " failed to sync keystroke. ret = " << ret;
				perror("Error ");
				exit(EXIT_FAILURE);
			}

		}
		if( (( nTemp_GPLEV = nPins_Level) &= nDown_Mask ) == 0 )//reset nTemp_GPLEV to the pre-bitwised value, and test for low bit (true, because Atari is active low)
		{
			cout << " down " << endl;
			memset(&Atari_Key, 0, sizeof(Atari_Key));//set aside a memory block and register a pointer

			Atari_Key.type = EV_KEY;//register event type, keystroke
			Atari_Key.code = KEY_DOWN;
			Atari_Key.value = 1;//press the key

			ret = write(fd, &Atari_Key, sizeof(Atari_Key));//write the event (keypress)

			if(ret < 0)//test for error code
			{
				cerr << " failed to create keyboard event. ret = " << ret;
				perror("Error keypress");
				exit(EXIT_FAILURE);
			}

			Atari_Key.type = EV_KEY;//release code
			Atari_Key.code = KEY_DOWN;
			Atari_Key.value = 0; //release the key

			ret = write(fd, &Atari_Key, sizeof(Atari_Key));//write the event (key release)

			if(ret < 0)//test for error code
			{
				cerr << " failed to create keyboard event. ret = " << ret;
				perror("Error ");
				exit(EXIT_FAILURE);
			}

	            	Atari_Key.type = EV_SYN;//synchronize keyboard event
	            	Atari_Key.code = 0;
            		Atari_Key.value = 0;

			ret = write(fd, &Atari_Key, sizeof(Atari_Key));//write the event (key synchronize)

			if(ret < 0)//test for error code
			{
				cerr << " failed to sync keystroke. ret = " << ret;
				perror("Error ");
				exit(EXIT_FAILURE);
			}

		}
		if( (( nTemp_GPLEV = nPins_Level ) &= nLeft_Mask ) == 0)//reset nTemp_GPLEV to the pre-bitwised value, and test for low bit (true, because Atari is active low)
		{
			cout << " left " << endl;

			memset(&Atari_Key, 0, sizeof(Atari_Key));//set aside a memory block and register a pointer

			Atari_Key.type = EV_KEY;//register event type, keystroke
			Atari_Key.code = KEY_LEFT;
			Atari_Key.value = 1;//press the key

			ret = write(fd, &Atari_Key, sizeof(Atari_Key));//write the event (keypress)

			if(ret < 0)//test for error code
			{
				cerr << " failed to create keyboard event. ret = " << ret;
				perror("Error keypress");
				exit(EXIT_FAILURE);
			}

			Atari_Key.type = EV_KEY;//release code
			Atari_Key.code = KEY_LEFT;
			Atari_Key.value = 0; //release the key

			ret = write(fd, &Atari_Key, sizeof(Atari_Key));//write the event (key release

			if(ret < 0)//test for error code
			{
				cerr << " failed to create keyboard event. ret = " << ret;
				perror("Error ");
				exit(EXIT_FAILURE);
			}

	            	Atari_Key.type = EV_SYN;//synchronize keyboard event
	            	Atari_Key.code = 0;
            		Atari_Key.value = 0;

			ret = write(fd, &Atari_Key, sizeof(Atari_Key));//write the event (key synchronize)

			if(ret < 0)//test for error code
			{
				cerr << " failed to sync keystroke. ret = " << ret;
				perror("Error ");
				exit(EXIT_FAILURE);
			}
		}
		else if( (( nTemp_GPLEV = nPins_Level ) &= nRight_Mask ) == 0)//reset nTemp_GPLEV to the pre-bitwised value, and test for low bit (true, because Atari is active low)
		{
			cout << " right " << endl;

			memset(&Atari_Key, 0, sizeof(Atari_Key));//set aside a memory block and register a pointer

			Atari_Key.type = EV_KEY;//register event type, keystroke
			Atari_Key.code = KEY_RIGHT;
			Atari_Key.value = 1;//press the key

			ret = write(fd, &Atari_Key, sizeof(Atari_Key));//write the event (keypress)

			if(ret < 0)//test for error code
			{
				cerr << " failed to create keyboard event. ret = " << ret;
				perror("Error keypress");
				exit(EXIT_FAILURE);
			}

			Atari_Key.type = EV_KEY;//release code
			Atari_Key.code = KEY_RIGHT;
			Atari_Key.value = 0; //release the key

			ret = write(fd, &Atari_Key, sizeof(Atari_Key));//write the event (key release

			if(ret < 0)//test for error code
			{
				cerr << " failed to create keyboard event. ret = " << ret;
				perror("Error ");
				exit(EXIT_FAILURE);
			}

	            	Atari_Key.type = EV_SYN;//synchronize keyboard event
	            	Atari_Key.code = 0;
            		Atari_Key.value = 0;

			ret = write(fd, &Atari_Key, sizeof(Atari_Key));//write the event (key synchronize)

			if(ret < 0)//test for error code
			{
				cerr << " failed to sync keystroke. ret = " << ret;
				perror("Error ");
				exit(EXIT_FAILURE);
			}
		}

		if(ctrl_c_pressed)//if the exit signal has been issued, must deallocate the uidevice before quitting
		{
			cout << " destroying uidevice " << endl;
			SNES.unmake_uiDevice();
			break;
		}
		
		//pass the "schmitt_delay" struct to the nano sleep method. Eliminates debouncing, and reduces processor usage by reducing speed of iteration.
		if( nanosleep( &schmitt_delay, NULL) < 0 )
		{
			cerr << "schmitt_delay failed, no debouncing of controller input. \n";
			perror("Error ");
		}

	}
	return 0;
}

//handles the ctrl-c signal issued by the Linux OS to terminate the infinite loop
void signal_handler(int signal)
{
    cerr << "CTRL + C pressed. Executing signal handler" << endl;
    ctrl_c_pressed = true;//set the control c flag
}
