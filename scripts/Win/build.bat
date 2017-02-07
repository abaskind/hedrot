@echo off
echo this builds the following binaries:
echo 	. the firmware (in firmware\build\hedrot-firmware.ino.hex
echo 	. the command-line demo (in command-line-demo/xcode/hedrotReceiverDemo) (64 bit only, 32 bit version easy to build as well)
echo 	. the max external hedrotReceiver.mxe (32 bit) and hedrotReceiver.mxe64 (64 bit)
echo --
echo The Standlone hedrotReceiver has to be rebuilt afterwards in Max!!!!
echo     the 32 bit version of the Standlone hedrotReceiver has to be built in the folder Max\standalone-Win\x86
echo     the 64 bit version of the Standlone hedrotReceiver has to be built in the folder Max\standalone-Win\x64
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

REM ######### build the Max external (32 and 64 bits) #############################
"%MSBuildPath%\MSBuild.exe" /t:Build "%rootDirectory%\Max\visual studio\hedrot_receiver.vcxproj" /p:CommonProgramFiles="C:\Program Files (x86)\Common Files" /p:PlatformToolset=v110 /p:Configuration=Release /p:Platform=Win32 /p:"VCTargetsPath=%VCTargetsPath%"
"%MSBuildPath%\MSBuild.exe" /t:Build "%rootDirectory%\Max\visual studio\hedrot_receiver.vcxproj" /p:CommonProgramFiles="C:\Program Files (x86)\Common Files" /p:PlatformToolset=v110 /p:Configuration=Release /p:Platform=x64 /p:"VCTargetsPath=%VCTargetsPath%"
