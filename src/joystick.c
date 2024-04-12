#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/joystick.h>

int main(void) {
	int in_joy_1, uinput;
	struct js_event event;
	struct axis_state axes[4] = {0};
	size_t axis;

	/**** This has to be changed for the autodetection and auto pairing ****/

	/* Opening the inputs */
	in_joy_1 = open("/dev/input/js1", O_RDONLY);

	if (joystick == -1) {
		perror("Could not open the input joystick 1!");
		return -1;
	}

	/* Opening the output */
	uinput = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

	if (uinput == -1) {
		perror("Could not open the output file!");
		return -1;
	}

	ioctl(fd, UI_SET_EVBIT, EV_KEY); // enable button/key handling
	ioctl(fd, UI_SET_KEYBIT, BTN_A);
	ioctl(fd, UI_SET_KEYBIT, BTN_B);
	ioctl(fd, UI_SET_KEYBIT, BTN_X);
	ioctl(fd, UI_SET_KEYBIT, BTN_Y);


	/* This exits when the controller is upnplugged */
	while 
