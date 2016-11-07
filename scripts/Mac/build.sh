#!/bin/sh
echo "this builds the following binaries:"
echo "	. the firmware (in firmware/build/hedrot-firmware.ino.hex"
echo "	. the command-line demo (in command-line-demo/xcode/hedrotReceiverDemo)"
echo "	. the max external hedrotReceiver.mxo"
echo
echo "The Standlone hedrotReceiver has to be rebuilt afterwards in Max!!!!"

thisDirectory=`pwd`
rootDirectory=$thisDirectory/../..

######### build firmware #############################
rm -rf $rootDirectory/firmware/build/*
ArduinoAppPath=/Applications/Arduino.app/Contents/MacOS/Arduino
$ArduinoAppPath -v --board teensy:avr:teensy31:usb=serial --pref build.path=$rootDirectory/firmware/build --verify $rootDirectory/firmware/hedrot-firmware/hedrot-firmware.ino

######### build the command-line demo #############################
xcodebuild -configuration Release -project $rootDirectory/command-line-demo/xcode/hedrotReceiverDemo.xcodeproj

######### build the Max external #############################
xcodebuild -configuration Deployment -project $rootDirectory/Max/xcode/hedrot_receiver.xcodeproj
