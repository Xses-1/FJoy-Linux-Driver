#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/joystick.h>

int main(void) {
	int joystick;
	struct js_event event;
	struct axis_state axes[4] = {0};
	size_t axis;

	/**** This has to be changed for the autodetection and auto pairing ****/
	joystick = open("/dev/input/js1", O_RDONLY);

	if (joystick == -1) {
		perror("Could not open the joystick!");
	}

	/* This exits when the controller is upnplugged 
