			how to build the binaries and create the distribution


1/ Mac OS X
	. edit « scripts/Mac/build.sh » and change the path to Arduino.app (variable ArduinoAppPath) if necessary
	. edit « Max/xcode/hedrot_receiver.xcconfig » and change the path to the « c74support » folder in the Max SDK (variable C74SUPPORT)
	. run « scripts/Mac/build.sh » from the command-line
	. open hedrotReceiver.maxproj in Max and build the standalone in the same directory
	. run « makeDistribution.sh » from the command-line

2/ Windows
	. edit « scripts/Win/build.bat » and change the path to arduino.exe (variable ArduinoAppPath) if necessary
	. edit "Max\visual studio\hedrot_receiver.props" and change the path to the « c74support » folder in the Max SDK (variable C74SUPPORT)
	. run « scripts/Win/build.bat » from the command-line (CAUTION: the script does not work Yet properly
	. open "command-line-demo\visual studio\hedrotReceiverDemo.sln" and build (Release/x64)
	. open "Max\visual studio\hedrot_receiver.sln" and build in Release for both platforms (Win32 and x64)
	. open hedrotReceiver.maxproj in Max and build the standalone in the same directory
	