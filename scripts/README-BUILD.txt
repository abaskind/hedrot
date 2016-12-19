			how to build the binaries and create the distribution


1/ Mac OS X
	. edit « scripts/Mac/build.sh » and change the path to Arduino.app (variable $ArduinoAppPath) if necessary
	. edit « Max/xcode/hedrot_receiver.xcconfig » and change the path to the « c74support » folder in the Max SDK (variable C74SUPPORT)
	. run « build.sh » from the command-line
	. open hedrotReceiver.maxproj in Max and build the standalone in the same directory
	. run « makeDistribution.sh » from the command-line

2/ Windows
	.
	. edit "Max\visual studio\hedrot_receiver.props" and change the path to the « c74support » folder in the Max SDK (variable C74SUPPORT)
	. 
	. open hedrotReceiver.maxproj in Max and build the standalone in the same directory
	. 