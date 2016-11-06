				hedrot-firmware

teensy firmware for the hedrot head tracker.

software requirements:
	. Arduino IDE 1.6.11
	. Teensyduino 1.30 (teensy support for the Arduino IDE). During the installation of teensyduino, at least the library i2c_t3 (i2c communication library optimized for teensy, standard with Teensyduino) has to be installed

hedrot-firmware can be compiled with the Arduino compiler: 
	=> open the file hedrot-firmware.ino
	=> in the Tools Menu, select as « Board » « Teensy 3.2/3.1 »
	=> compile
	the hex file of the firmware binary has then to be found manually (look at the Arduino console)

alternatively, the firmware can be compiled from the command line:

/Applications/Arduino.app/Contents/MacOS/Arduino -v --board teensy:avr:teensy31:usb=serial --pref build.path=$FIRMWARE_FOLDER/build --verify $FIRMWARE_FOLDER/hedrot-firmware/hedrot-firmware.ino

... where $FIRMWARE_FOLDER is an environment variable corresponding to the current folder. The main advantage of the command line method is that the hex file hedrot-firmware.ino.hex can be now easily accessed from the "build" subfolder 