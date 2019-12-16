# hedrot

### Note: the development of hedrot has been stopped and the project is currently not maintained.

## What is hedrot?
Hedrot (for "head rotation tracker") is a low-cost (around 25 euros with a teensy LC) and efficient open-source hardware/software solution for head tracking. Hedrot is especially suitable for binaural rendering (3D-Audio on headphones), and has been initially designed for use with the binaural renderer Bipan as part of the Bili Project http://www.bili-project.org/.

Hedrot provides an estimation of the rotation of the sensor (thus of the head if the sensor is attached to headphones) for the most usual x-y-z coordinate systems, either as a quaternion, or as a set of 3 orientation angles yaw, pitch and roll with two different orders (yaw-pitch-roll or roll-pitch-yaw). The main application provided with the distribution, hedrotReceiver, sends this information as OSC streams, with the extra possibility to scale each stream independently.

Contrary to several generic open-source head tracking solutions, hedrot relies on and has been optimized for specific widely spread and efficient hardware parts, i.e. a Teensy 3 or Teensy LC board (optimized Arduino-like board) combined to a IMU/MARG daughter board with 3 common sensors (Analog Devices ADXL345 accelerometer, Honeywell HMC5883L magnetometer and Invensense ITG-3200 gyroscope). 

The estimation algorithm is based on a modified version of the precise and efficient open-source gradient descent algorithm from Sebastian Madgwick. The technology was dramatically optimized for speed: the head tracker can deliver data at rates up to 2 kHz. The hardware latency of the Teensy board and USB communication relies below 2 ms. The overall latency (including sensor latency and time constant of the algorithm) is being currently measured. 

## Licensing and Credits
The first development phase of Hedrot has achieved in collaboration with the Conservatoire National Supérieur de Musique et de Danse de Paris (http://www.conservatoiredeparis.fr/) as part of the "Bili" project (http://www.bili-project.org/).

Matthieu Aussal (CMAP - Ecole Polytechnique / CNRS) contributed to new calibration routines (version > 1.2)

Hedrot is licensed under the terms of the GNU General Public License (version 3) as published by the Free Software Foundation.

Part of code is derived from Sebastian Madgwick's open-source gradient descent angle estimation algorithm (http://x-io.co.uk/open-source-imu-and-ahrs-algorithms/)


### Developers and Contributors
* Alexis Baskind (sound engineer, main developer)
* Jean-Christophe Messonnier (sound engineer)
* Jean-Marc Lyzwa (sound engineer)
* Matthieu Aussal, CMAP - Ecole Polytechnique / CNRS (calibration routines)


## Hardware Requirements
- Head tracker parts:
  - 1 Teensy 3 board (tested with versions 3.1, 3.2 and LC)
  - 1 USB to Micro-USB 2.0 cable, minimum length 1.5 m
  - 1 gy85 IMU daughter board with the following sensors:
    - 1 Analog Devices ADXL345 accelerometer
    - 1 Honeywell HMC5883L compass (magnetometer)
    - 1 Invensense ITG3200 gyroscope

## Software Requirements
- Mac OS 10.9.5 or later
- Windows: Windows 7 or later
- the teensy.app firmware flash loader
- Windows: Microsoft Visual C++ 2012 Redistributable package (x86 or x64 version, depending on if a 32 or 64 bit version of the hedrot is being used)
- Windows: Teensy serial driver (called "Windows Serial Installer" on https://www.pjrc.com)

## Extra Requirements for building from sources:
- Xcode version 8.2 or later (for Mac), or Visual Studio 2012 (for Windows)
- Windows: a windows version of awk (like gawk) and the program 7-zip to make the package
- Max version 6 at least (to rebuild the "hedrotReceiver" application. Not needed otherwise)
- Arduino IDE 1.8.3
- Windows: a distribution of BLAS, Lapack and LAPACKE. See README-BUILD.txt for more information about how to build it
- Teensyduino 1.37 (teensy support for the Arduino IDE and the teensy USB serial driver on Windows), with at least the i2c_t3 library. Note: Teensyduino already includes the Teensy serial driver on Windows

## Project organisation
- the empty folder "build" is where the distribution will be built
- the folder "command-line-demo" contains the sources of the command-line demonstration
- the folder "doc" contains the documentation
- the folder "examples" contains application examples
- the folder "firmware" contains the sources of the firmware to be uploaded in the teensy board
- the folder "libhedrot" contains the sources of the receiver library
- the folder "matlab" contains programs for matlab and octave
- the folder "Max" contains the sources of the main receiver application, written in Max
- the folder "scripts" contains the scripts for building binaries and the distribution
