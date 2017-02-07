@echo off
echo This script builds a 64 bit windows distribution of hedrot (including however the 32 bit version of the max object). 
echo Before running this script, be sure that: 
echo    1/ all binaries have been compiled
echo    2/ a pdf version of the doc has been created
echo    3/ the Max standalone hedrotReceiver has been built.

set today=%date:~-2%-%date:~3,2%-%date:~0,2%

set thisDirectory=%cd%
set "rootDirectory=%thisDirectory%\..\.."
set "destDirectory=%rootDirectory%\build\forPackaging"

set "testfile=%rootDirectory%\libhedrot\libhedrot.h"
awk "/\#define HEDROT_VERSION / { print $3 }" %testfile% > tmp.txt
sed -i 's/"//g' tmp.txt
set /p hedrotVersion=<tmp.txt
rm tmp.txt
set "hedrotVersionMessage=hedrot version: %hedrotVersion%"
echo %hedrotVersionMessage%

set "testfile=%rootDirectory%\firmware\hedrot-firmware\hedrot_comm_protocol.h"
awk "/\#define HEDROT_FIRMWARE_VERSION / { print $3 }" %testfile% > tmp.txt
sed -i 's/"//g' tmp.txt
set /p hedrotFirmwareVersion=<tmp.txt
rm tmp.txt
set "hedrotFirmwareVersionMessage=Headtracker Firmware version: %hedrotFirmwareVersion%"
echo %hedrotFirmwareVersionMessage%

set "packageName=hedrot-%hedrotVersion%-%today%-Win64"
set "packageFolder=%destDirectory%\tmpPackageFolder\%packageName%"


REM ######### create destination folder #############################
rmdir /s /q "%destDirectory%\tmpPackageFolder"
md "%packageFolder%"

REM ######### prepare and copy Max Standalone #############################

REM #copy the standalone and the default preset file
md "%packageFolder%\hedrotReceiver"
xcopy "%rootDirectory%\Max\standalone-Win\x64\hedrotReceiver" "%packageFolder%\hedrotReceiver" /Y /Q /i /s
copy "%rootDirectory%\Max\hedrotReceiver.json" "%packageFolder%\hedrotReceiver" > nul


REM ######### copy the command-line-demo #############################
md "%packageFolder%\command-line-demo"
copy "%rootDirectory%\command-line-demo\visual studio\exe\Release_x64\hedrotReceiverDemo.exe" "%packageFolder%\command-line-demo\" > nul 

REM ######### copy the firmware #############################
md "%packageFolder%\firmware"
copy "%rootDirectory%\firmware\build\hedrot-firmware.ino.hex" "%packageFolder%\firmware\hedrot-firmware_version_%hedrotFirmwareVersion%.hex" > nul
copy "%rootDirectory%\firmware\CHANGELOG.txt" "%packageFolder%\firmware\" > nul

REM ######### copy the max source code and external #############################
md "%packageFolder%\Max"
copy "%rootDirectory%\Max\hedrotReceiver.json" "%packageFolder%\Max\" > nul
copy "%rootDirectory%\Max\patches\*.maxpat" "%packageFolder%\Max\" > nul
copy "%rootDirectory%\Max\hedrot_receiver.mxe" "%packageFolder%\Max\" > nul
copy "%rootDirectory%\Max\hedrot_receiver.mxe64" "%packageFolder%\Max\" > nul

REM ######### copy the doc #############################
copy "%rootDirectory%\doc\hedrot user manual.pdf" "%packageFolder%" > nul

REM ######### copy the license file #############################
copy "%rootDirectory%\LICENSE" "%packageFolder%" > nul

REM ######### copy the CHANGELOG.txt file #############################
copy "%rootDirectory%\CHANGELOG.txt" "%packageFolder%" > nul

REM ######### copy and update the main README.txt file #############################
copy "%rootDirectory%\scripts\README-DISTRIBUTION.txt" "%packageFolder%\README.txt" > nul
sed -i "s/Hedrot Version: XXXX/Hedrot Version: %hedrotVersion%/g;" %packageFolder%\README.txt
sed -i "s/Firmware Version: YYYY/Firmware Version: %hedrotFirmwareVersion%/g;" %packageFolder%\README.txt


REM ######### ZIP #############################

REM #erase the ZIP if it exists
del "%rootDirectory%\build\%packageName%.zip"

REM #create the ZIP
cd "%destDirectory%\tmpPackageFolder\%packageName%"
7z a -r -tzip "%rootDirectory%\build\%packageName%.zip" * > nul

REM #erase temp build directory
cd %destDirectory%
rmdir /s /q "%destDirectory%\tmpPackageFolder"

cd %thisDirectory%