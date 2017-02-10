			how to build the binaries and create the distribution


1/ Mac OS X
	. edit « scripts/Mac/build.sh » and change the path to Arduino.app (variable ArduinoAppPath) if necessary
	. edit « Max/xcode/hedrot_receiver.xcconfig » and change the path to the « c74support » folder in the Max SDK (variable C74SUPPORT)
	. run « scripts/Mac/build.sh » from the command-line
	. open hedrotReceiver.maxproj in Max and build the standalone in the same directory
	. run « makeDistribution.sh » from the command-line

2/ Windows
	. edit « scripts/Win/build.bat » and change the following paths if necessary:
			- path to arduino.exe (variable ArduinoAppPath)
			- path to Microsoft.NET framework (variable MSBuildPath)
			- path to Visual Studio target V110 (variable VCTargetsPath)
	. edit "Max\visual studio\hedrot_receiver.props" and change the path to the « c74support » folder in the Max SDK (variable C74SUPPORT)
	. run « scripts/Win/build.bat » 
	. open hedrotReceiver.maxproj in Max and build the standalone in the same directory
	. run « scripts/Win/makeDistribution.bat »
	