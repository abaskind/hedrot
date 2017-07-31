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
set "ArduinoAppPath=C:\Program Files (x86)\Arduino\arduino.exe"
set "MSBuildPath=C:\Windows\Microsoft.NET\Framework\v4.0.30319"
set "VCTargetsPath=C:\Program Files (x86)\MSBuild\Microsoft.Cpp\v4.0\V110"


REM ######### build firmware #############################
rm -rf "%rootDirectory%\firmware\build\*"
"%ArduinoAppPath%" -v --board teensy:avr:teensy31:usb=serial --pref "build.path=%rootDirectory%\firmware\build" --verify "%rootDirectory%\firmware\hedrot-firmware\hedrot-firmware.ino"

REM ######### build the command-line demo #############################
"%MSBuildPath%\MSBuild.exe" /t:Build "%rootDirectory%\command-line-demo\visual studio\hedrotReceiverDemo.vcxproj" /p:CommonProgramFiles="C:\Program Files (x86)\Common Files" /p:PlatformToolset=v110 /p:Configuration=Release /p:Platform=x64 /p:"VCTargetsPath=%VCTargetsPath%"

REM ######### build the Max external #############################
"%MSBuildPath%\MSBuild.exe" /t:Build "%rootDirectory%\hedrotReceiver\visual studio\hedrot_receiver.vcxproj" /p:CommonProgramFiles="C:\Program Files (x86)\Common Files" /p:PlatformToolset=v110 /p:Configuration=Release /p:Platform=x64 /p:"VCTargetsPath=%VCTargetsPath%"
