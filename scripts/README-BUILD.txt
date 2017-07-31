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
	. edit « scripts/Win/makeDistribution.bat »and change the following paths if necessary:
			- path to rcedit (small tool to change the app icon, available at https://github.com/electron/rcedit)
	. run « scripts/Win/makeDistribution.bat »

In order to build hedrot binaries on Windows, a distribution of BLAS+LAPACK+LAPACKE is required. To build this distribution:

. install MinGW for 64 bits. It can be found at: http://mingw-w64.org

. install cmake version 2.8.12 (https://cmake.org/files/). CAUTION: lapack does not build with cmake v3+, build it with 2.8.12

. download lapack sources from github (https://github.com/Reference-LAPACK). CAUTION: the latest release of the sources (3.7.0) does not build in 64 bits. The bug has been corrected afterwards (29 dec 2016). Download the repository version from 29 dec 2016, only this one will build. Here is the link:
https://github.com/Reference-LAPACK/lapack-release/archive/42944f6fdee4de98fd1dd650c4f81047937635c2.zip

. follow the instructions in http://icl.cs.utk.edu/lapack-for-windows/lapack/#build


. in the visual studio project:
	1/ add the directory containing the lapacke headers in Project Properties => Configuration Properties => C/C++ => General => Additional	Include Directories
	2/ add the directory containing the BLAS+lapack+lapacke .lib files in Project Properties => Configuration Properties => Linker => General => Additional Library Directories
	
	
. the following DLLs built in the lapack sources are required for running the program: libblas.dll, liblapack.dll, liblapacke.dll.
	

. copy all those dlls, as well as libgfortran-3.dll and libgcc_s_seh-1.dll, libquadmath-0.dll and libwinpthread-1.dll from the MinGW64 distribution close to the binary, i.e.:
	- for the command-line demo: command-line-demo\visual studio\exe\Release_x64
	- for the max external: in directory Max\

	