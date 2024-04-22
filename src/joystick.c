#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <linux/uinput.h>
#include <sys/select.h>

static void setup_abs(int fd, unsigned chan, int min, int max);
void print_js_event(struct js_event event);
void print_uin_event (struct input_event uin_ev);
void lock_file(int fd, struct flock* lock); 
void unlock_file(int fd, struct flock* lock);

int main(int argc, char* argv[]) {
	int in_joy_1, in_joy_2, uinput;
	fd_set set;

	struct timeval timeout;
	struct uinput_setup usetup;
	struct js_event js_ev1;
	struct js_event js_ev2;
	struct flock lock;

	timeout.tv_sec = 0;
	timeout.tv_usec = 10;

	lock.l_whence = SEEK_SET; /* Lock the whole file */
    lock.l_start = 0;
    lock.l_len = 0; /* Lock the whole file */

	/* Opening the inputs */
	if (argc < 3) {
		perror("Too few arguments!");
		return -1;
	}

	/* 1 is left 2 is right */
	in_joy_1 = open(argv[1], O_RDONLY);
	in_joy_2 = open(argv[2], O_RDONLY);

	if (in_joy_1 == -1) {
		perror("Could not open the input joystick 1!");
		close(in_joy_1);
		close(in_joy_2);
		return -1;
	}

	if (in_joy_2 == -1) {
		perror("Could not open the input joystick 2!");
		close(in_joy_1);
		close(in_joy_2);
		return -1;
	}

	lock_file(in_joy_1, &lock);
	lock_file(in_joy_2, &lock);
	// I assume that I will never double lock them 
	lock.l_type = F_UNLCK;


	/* Opening the output */
	uinput = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

	if (uinput < 0) {
		perror("Could not open the output file!");
		unlock_file(in_joy_1, &lock);
		unlock_file(in_joy_2, &lock);
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
		unlock_file(in_joy_1, &lock);
		unlock_file(in_joy_2, &lock);
		close(uinput);
		return -1;
    }
  
	if (ioctl(uinput, UI_DEV_CREATE)) {
		perror("UI_DEV_CREATE failed!");
		unlock_file(in_joy_1, &lock);
		unlock_file(in_joy_2, &lock);
		close(uinput);
		return -1;
    }


	/* This exits when the controller is upnplugged */
	while(1) {  
		struct input_event uin_ev[2];
		memset(&uin_ev, 0, sizeof(uin_ev));

		int data_available_js1, data_available_js2;
		data_available_js1 = 0;
		data_available_js2 = 0;

		FD_ZERO(&set);
		FD_SET(in_joy_1, &set);
		int rv = select(in_joy_1 + 1, &set, NULL, NULL, &timeout);

		if (rv == -1) {
			perror("Select 1 broky!");

		} else if (rv != 0) {
			if (read(in_joy_1, &js_ev1, sizeof(js_ev1)) != sizeof(js_ev1)) {
				perror("End of the input stream!");
				unlock_file(in_joy_1, &lock);
				unlock_file(in_joy_2, &lock);
				close(uinput);
				return 0;       
			}

			data_available_js1 = 1;
		}

		FD_ZERO(&set);
		FD_SET(in_joy_2, &set);
		rv = select(in_joy_2 + 1, &set, NULL, NULL, &timeout);

		if (rv == -1) {
			perror("Select 2 broky!");

		} else if (rv != 0) {
			if (read(in_joy_2, &js_ev2, sizeof(js_ev2)) != sizeof(js_ev2)) {
				perror("End of the input stream!");
				unlock_file(in_joy_1, &lock);
				unlock_file(in_joy_2, &lock);
				close(uinput);
				return 0;       
			}

			data_available_js2 = 1;
		}


		if (data_available_js1 == 1) {
			//print_js_event(js_ev1);

			/* Here the mapping happens that converts the
			 * js events to uinput events and does any
			 * layout changes
			 */
			if (js_ev1.type == JS_EVENT_BUTTON) {
				uin_ev[0].type = EV_KEY;

				/* This has to be adjuste for a specific mapping */
				switch (js_ev1.number) {
					case 0:
						uin_ev[0].code = BTN_DPAD_DOWN;
						break;   
								 
					case 1:      
						uin_ev[0].code = BTN_DPAD_RIGHT;
						break;   
								 
					case 2:      
						uin_ev[0].code = BTN_DPAD_LEFT;
						break;   
								 
					case 3:      
						uin_ev[0].code = BTN_DPAD_UP;
						break;   
								 
					case 4:      
						uin_ev[0].code = BTN_TL;
						break;   
								 
					case 6:      
						uin_ev[0].code = BTN_THUMBL;
						break;   
								 
					default:     
						break;
				}

				uin_ev[0].value = (__s32)js_ev1.value;
				
			} else if (js_ev1.type == JS_EVENT_AXIS) {
				uin_ev[0].type = EV_ABS;

				if (js_ev1.number == 0) {
					uin_ev[0].code = ABS_X;

				} else if (js_ev1.number == 1) {
					uin_ev[0].code = ABS_Y;
				}

				uin_ev[0].value = (__s32)js_ev1.value;
			}

			uin_ev[1].type = EV_SYN;
			uin_ev[1].code = SYN_REPORT;
			uin_ev[1].value = 0;


			if (write(uinput, &uin_ev, sizeof(uin_ev)) < 0) {
				perror("Could not write to the output!");
				unlock_file(in_joy_1, &lock);
				unlock_file(in_joy_2, &lock);
				close(uinput);
				return -1;
			}

			fflush(stdout);
		}


		if (data_available_js2 == 1) {
			//print_js_event(js_ev2);

			if (js_ev2.type == JS_EVENT_BUTTON) {
				uin_ev[0].type = EV_KEY;

				/* This has to be adjuste for a specific mapping */
				switch (js_ev2.number) {
					case 0:
						uin_ev[0].code = BTN_A;
						break;   
								 
					case 1:      
						uin_ev[0].code = BTN_B;
						break;   
								 
					case 2:      
						uin_ev[0].code = BTN_X;
						break;   
								 
					case 3:      
						uin_ev[0].code = BTN_Y;
						break;   
								 
					case 5:      
						uin_ev[0].code = BTN_TR;
						break;   
								 
					case 7:      
						uin_ev[0].code = BTN_THUMBR;
						break;   
								 
					default:     
						break;
				}

				uin_ev[0].value = (__s32)js_ev2.value;
				
			} else if (js_ev2.type == JS_EVENT_AXIS) {
				uin_ev[0].type = EV_ABS;

				if (js_ev2.number == 0) {
					uin_ev[0].code = ABS_RX;

				} else if (js_ev2.number == 1) {
					uin_ev[0].code = ABS_RY;
				}

				uin_ev[0].value = (__s32)js_ev2.value;
			}

			uin_ev[1].type = EV_SYN;
			uin_ev[1].code = SYN_REPORT;
			uin_ev[1].value = 0;


			if (write(uinput, &uin_ev, sizeof(uin_ev)) < 0) {
				perror("Could not write to the output!");
				unlock_file(in_joy_1, &lock);
				unlock_file(in_joy_2, &lock);
				close(uinput);
				return -1;
			}

			fflush(stdout);
		}

		/* The thing is using 100% of cpu, so this might help */
		usleep(100);
	}

	if (ioctl(uinput, UI_DEV_DESTROY)) {
		unlock_file(in_joy_1, &lock);
		unlock_file(in_joy_2, &lock);
		close(uinput);
		return 1;
	}

	unlock_file(in_joy_1, &lock);
	unlock_file(in_joy_2, &lock);
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


/* This can be used for testing and troubleshooting */
void print_js_event (struct js_event event) {
	switch (event.type) {
		case JS_EVENT_BUTTON:
			printf("Event type: %u; Event number: %u; Event value: %u; \n",
					event.type, event.number, event.value);
			break;

		case JS_EVENT_AXIS:
			printf("Event type: %u; Axis number: %u; Axis value: %u; \n",
					event.type, event.number, event.value);
			break;

		default:
			/* Ignore init events. */
			break;
	}
}

/* This can be used for testing and troubleshooting */
void print_uin_event (struct input_event uin_ev) {
	switch (uin_ev.type) {
		case EV_KEY:
			printf("Event type: %u; Event code %u; Event value: %u; \n",
					uin_ev.type, uin_ev.code, uin_ev.value);
			break;

		case JS_EVENT_AXIS:
			printf("Event type: %u; Axis number: %u; Axis value: %u; \n",
					uin_ev.type, uin_ev.code, uin_ev.value);
			break;

		default:
			break;
	}
}

/* File locking */
void lock_file(int fd, struct flock* lock) {
    lock->l_type = F_RDLCK; /* Exclusive lock */

    if (fcntl(fd, F_SETLK, lock) == -1) {
        perror("fcntl");
    }
}

/* File unlocking and releasing */
void unlock_file(int fd, struct flock* lock) {
    lock->l_type = F_UNLCK;

    if (fcntl(fd, F_SETLK, lock) == -1) {
        perror("fcntl");
    }

	close(fd);
}
