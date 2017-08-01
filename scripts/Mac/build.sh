#!/bin/sh
echo "this builds the following binaries:"
echo "	. the firmware (in firmware/build/hedrot-firmware.ino.hex"
echo "	. the command-line demo (in command-line-demo/xcode/hedrotReceiverDemo)"
echo "	. the max external hedrotReceiver.mxo"
echo
echo "The Standlone hedrotReceiver has to be rebuilt afterwards in Max!!!!"

thisDirectory=`pwd`
rootDirectory=$thisDirectory/../..

ArduinoAppPath=/Applications/Arduino.app/Contents/MacOS/Arduino

######### build firmware (both versions) #############################
rm -rf $rootDirectory/firmware/build/*

$ArduinoAppPath -v --board teensy:avr:teensy31:usb=serial,speed=96,opt=o1std,keys=en-us --pref build.path=$rootDirectory/firmware/build --verify $rootDirectory/firmware/hedrot-firmware/hedrot-firmware.ino
cp $rootDirectory/firmware/build/hedrot-firmware.ino.hex $rootDirectory/firmware/hex/hedrot-firmware-teensy31-32.ino.hex 

$ArduinoAppPath -v --board teensy:avr:teensyLC:usb=serial,speed=48,opt=osstd,keys=en-us --pref build.path=$rootDirectory/firmware/build --verify $rootDirectory/firmware/hedrot-firmware/hedrot-firmware.ino
cp $rootDirectory/firmware/build/hedrot-firmware.ino.hex $rootDirectory/firmware/hex/hedrot-firmware-teensyLC.ino.hex 

######### build the command-line demo #############################
xcodebuild -configuration Release -project $rootDirectory/command-line-demo/xcode/hedrotReceiverDemo.xcodeproj

######### build the Max external #############################
xcodebuild -configuration Deployment -project $rootDirectory/hedrotReceiver/xcode/hedrot_receiver.xcodeproj
