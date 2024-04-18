#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <linux/uinput.h>

static void setup_abs(int fd, unsigned chan, int min, int max);
int read_event(int fd, struct js_event *event);
void print_event(struct js_event event);

int main(void) {
	int in_joy_1, /*in_joy_2,*/ uinput;
	//struct axis_state axes[4] = {0};
	struct uinput_setup usetup;
	struct js_event ev;

	/**** This has to be changed for the autodetection and auto pairing ****/

	/* Opening the inputs */
	in_joy_1 = open("/dev/input/js1", O_RDONLY);

	if (in_joy_1 == -1) {
		perror("Could not open the input joystick 1!");
		close(in_joy_1);
		return -1;
	}

	/* Opening the output */
	uinput = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

	if (uinput < 0) {
		perror("Could not open the output file!");
		close(in_joy_1);
		close(uinput);
		return -1;
	}


	/* Initializing the possible output calls */
	ioctl(uinput, UI_SET_EVBIT, EV_KEY); // enable button/key handling

	ioctl(uinput, UI_SET_KEYBIT, BTN_A);
	ioctl(uinput, UI_SET_KEYBIT, BTN_B);
	ioctl(uinput, UI_SET_KEYBIT, BTN_X);
	ioctl(uinput, UI_SET_KEYBIT, BTN_Y);

	ioctl(uinput, UI_SET_KEYBIT, BTN_TL);
	ioctl(uinput, UI_SET_KEYBIT, BTN_TR);
	ioctl(uinput, UI_SET_KEYBIT, BTN_THUMBL);
	ioctl(uinput, UI_SET_KEYBIT, BTN_THUMBR);

	ioctl(uinput, UI_SET_KEYBIT, BTN_DPAD_UP);
	ioctl(uinput, UI_SET_KEYBIT, BTN_DPAD_DOWN);
	ioctl(uinput, UI_SET_KEYBIT, BTN_DPAD_LEFT);
	ioctl(uinput, UI_SET_KEYBIT, BTN_DPAD_RIGHT);

	ioctl(uinput, UI_SET_EVBIT, EV_ABS); // enable analog absolute position handling
  
	setup_abs(uinput, ABS_X,  -32767, 32767);
	setup_abs(uinput, ABS_Y,  -32767, 32767);

	setup_abs(uinput, ABS_RX, -32767, 32767);
	setup_abs(uinput, ABS_RY, -32767, 32767);

	memset(&usetup, 0, sizeof(usetup));
	usetup.id.bustype = BUS_USB;
	usetup.id.vendor = 0x0042; /* sample vendor */
	usetup.id.product = 0x0037; /* sample product */
	strcpy(usetup.name, "FJoys Emulated Unified Output");

	if (ioctl(uinput, UI_DEV_SETUP, &usetup)) {
      perror("UI_DEV_SETUP failed!");
	  close(in_joy_1);
	  close(uinput);
      return -1;
    }
  
	if (ioctl(uinput, UI_DEV_CREATE)) {
      perror("UI_DEV_CREATE failed!");
	  close(in_joy_1);
	  close(uinput);
      return -1;
    }


	/* This exits when the controller is upnplugged */
	while(read_event(in_joy_1, &ev) == 0) {
		print_event(ev);

		/*
		if (write(uinput, &ev, sizeof ev) < 0) {
			perror("Could not write to the output!");
			close(in_joy_1);
			close(uinput);
			return -1;
		}*/

		fflush(stdout);
	}

	close(in_joy_1);
	close(uinput);
	return 0;
}

static void setup_abs(int uinput, unsigned chan, int min, int max) {
	if (ioctl(uinput, UI_SET_ABSBIT, chan)) {
		perror("UI_SET_ABSBIT failed!");
	}

	struct uinput_abs_setup s = {
		.code = chan,
		.absinfo = {.minimum = min, .maximum = max},
	};

	if (ioctl(uinput, UI_ABS_SETUP, &s)) {
		perror("UI_ABS_SETUP failed!");
	}
}

int read_event(int fd, struct js_event *event) {
    ssize_t bytes;

    bytes = read(fd, event, sizeof(*event));

    if (bytes == sizeof(*event))
        return 0;

    /* Error, could not read full event. */
    return -1;
}

/* This can be used for testing and troubleshooting */
void print_event(struct js_event event) {
	switch (event.type) {
		case JS_EVENT_BUTTON:
			printf("Event type: %u; Event number: %u; Event value: %u; \n",
					event.type, event.number, event.value);
			break;

		/*
		case JS_EVENT_AXIS:
			axis = get_axis_state(&event, axes);
			if (axis < 3) {
				printf("Axis %zu at (%6d, %6d)\n", axis, axes[axis].x, axes[axis].y);
			}
			break;
			*/

		default:
			/* Ignore init events. */
			break;
	}
}
