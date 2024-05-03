# FJoy-Linux-Driver
Linux driver for Framework laptop joysitcks project that unifies the 2 devices into one and emulates the typical layout of an XBox controller.

## Files
### Executable
If you are just interested in the final product then FJoys\_driver is all you need. You can run it simply by double clicking it in the file explorer of your distribution, or run it from the command line:

```
./FJoys_driver &
```
It will exit when the joysticks are unpluged, so remember to run it again if you swap the expansion cards.


### Compiling from source
Src contains the source code file joystick.c. To compile it you are going to need the following dependencies: 

```
make
gcc
```
You can simply compile it by running: 
```
make
```
If you want to get rid of the executable then do: 
```
make clean
```

## The joystick.c structure
The general idea behind this driver is to read the raw inputs of 2 joysticks from /dev/jsx, map them to standard XBox like controller output and write to /dev/uinput file that allows to create an emulated joystick device.<br> 

In the joystick.c file, a the begginig all the necessary libraries are included, then the prototypes of the functions are defined, and all the necessary variables and data structures are declared. After that, the programm checks which joystick is left and which is right on the lines 41, and 42. The function getJoystickNumber simply opens the /proc/bus/input/devices file that lists all the informations about the connected input devices, this is used to find a line with the substring specified as the argument and then find the first line containing /dev/jsx after it to check to which jsx is the device matched. The assigment of jsx files will depend on which order were the joystick plugged in.<br> 

After the files has been identified the program tries to open them and checks for all the possible errors. After that it locks the opened files, so other programs should not be able to read it, since all we gonna need now is only the unified file.<br> 

As next the uinput file is opened, all of its possible output calls are initialized, and a new virtual joystick device is set through it.<br> 

Finally the endless while loop is called. It checks in a loop if the data are available in the joystick files and sets the corresponding data\_available\_jsx flag. Then the the js events are mapped to the uinput events and the 2 joysticks layouts are remapped to one standard layout. Everything is repeated twice for 2 devices. <br>

If any error occurs then the files are closed and the program exits.<br>

More on the js device and uinput can be found in the Linux kernel source code <a href="https://github.com/torvalds/linux/blob/master/include/uapi/linux/uinput.h">here</a> and <a href="https://github.com/torvalds/linux/blob/master/include/uapi/linux/input.h">here</a>.<a href="https://kernel.org/doc/html/v4.12/input/uinput.html">This manual page also provides some useful information.</a> <br>
