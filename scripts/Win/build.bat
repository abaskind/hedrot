@echo off
echo this builds the following binaries (in 64 bits only):
echo 	. the firmware (in firmware\build\hedrot-firmware.ino.hex
echo 	. the command-line demo (in command-line-demo/xcode/hedrotReceiverDemo) 
echo 	. the max external hedrotReceiver.mxe64
echo --
echo The Standlone hedrotReceiver has to be rebuilt afterwards in Max!!!!
echo     the 64 bit version of the Standlone hedrotReceiver has to be built in the folder hedrotReceiver\standalone-Win\x64
echo --

set thisDirectory=%cd%
set "rootDirectory=%thisDirectory%\..\.."

REM  ######### PATH SETTINGS #############################
set "ArduinoAppPath=C:\Program Files (x86)\Arduino\arduino_debug.exe"
set "MSBuildPath=C:\Windows\Microsoft.NET\Framework\v4.0.30319"
set "VCTargetsPath=C:\Program Files (x86)\MSBuild\Microsoft.Cpp\v4.0\V110"


REM ######### build firmware (both versions) #############################
rm -rf "%rootDirectory%\firmware\build\*"

"%ArduinoAppPath%" --verify --board teensy:avr:teensy31:usb=serial,speed=96,opt=o1std,keys=en-us --pref "build.path=%rootDirectory%\firmware\build" "%rootDirectory%\firmware\hedrot-firmware\hedrot-firmware.ino"
copy "%rootDirectory%\firmware\build\hedrot-firmware.ino.hex" "%rootDirectory%\firmware\hex\hedrot-firmware-teensy31-32.ino.hex"

"%ArduinoAppPath%" --verify --board teensy:avr:teensyLC:usb=serial,speed=48,opt=osstd,keys=en-us --pref "build.path=%rootDirectory%\firmware\build" "%rootDirectory%\firmware\hedrot-firmware\hedrot-firmware.ino"
copy "%rootDirectory%\firmware\build\hedrot-firmware.ino.hex" "%rootDirectory%\firmware\hex\hedrot-firmware-teensyLC.ino.hex"


REM ######### build the command-line demo #############################
"%MSBuildPath%\MSBuild.exe" /t:Build "%rootDirectory%\command-line-demo\visual studio\hedrotReceiverDemo.vcxproj" /p:CommonProgramFiles="C:\Program Files (x86)\Common Files" /p:PlatformToolset=v110 /p:Configuration=Release /p:Platform=x64 /p:"VCTargetsPath=%VCTargetsPath%"

REM ######### build the Max external #############################
"%MSBuildPath%\MSBuild.exe" /t:Build "%rootDirectory%\hedrotReceiver\visual studio\hedrot_receiver.vcxproj" /p:CommonProgramFiles="C:\Program Files (x86)\Common Files" /p:PlatformToolset=v110 /p:Configuration=Release /p:Platform=x64 /p:"VCTargetsPath=%VCTargetsPath%"
