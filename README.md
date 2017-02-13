# hedrot

## What is hedrot?
Hedrot (for "head rotation tracker") is a low-cost and efficient open-source hardware/solution for head tracking. Contrary to several generic open-source head tracking solutions, hedrot relies on and has been optimized for specific widely spread and efficient hardware parts, i.e. a Teensy 3 board (optimized Arduino-like board) combined to a IMU/MARG daughter board with 3 common sensors (Analog Devices ADXL345 accelerometer, Honeywell HMC5883L magnetometer and Invensense ITG-3200 gyroscope). 

The estimation algorithm is based on a modified version of the precise and efficient open-source gradient descent algorithm from Sebastian Madgwick. The technology was dramatically optimized for speed: the head tracker can deliver data at a rates up to 2 kHz. The hardware latency of the Teensy board and USB communication relies below 2 ms. The overall latency (including sensor latency and time constant of the algorithm) was estimated to remain between 10 and 15 ms for typical settings, further experiments are currently being done to measure it. 

## Licensing and Credits
The first development phase of Hedrot has achieved in collaboration with the Conservatoire National Supérieur de Musique et de Danse de Paris (http://www.conservatoiredeparis.fr/) as part of the "Bili" project (http://www.bili-project.org/).

Hedrot is licensed under the terms of the GNU General Public License (version 3) as published by the Free Software Foundation.

Part of code is derived from Sebastian Madgwick's open-source gradient descent angle estimation algorithm (http://x-io.co.uk/open-source-imu-and-ahrs-algorithms/)
Part of code is derived from "comport", (c) 1998-2005  Winfried Ritsch, Institute for Electronic Music - Graz.
Part of code is derived from Yuri Petrov "ellipsoid fit" algorithm (initially written for Matlab).


### Developers and Contributors
* Alexis Baskind (sound engineer, main developer)
* Jean-Christophe Messonnier (sound engineer)
* Jean-Marc Lyzwa (sound engineer)

## Hardware Requirements
- Head tracker parts:
  - 1 Teensy 3 board (tested with versions 3.1 and 3.2)
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
- Xcode version 6.2 or later(for Mac), or Visual Studio 2012 (for Windows)
- Windows: a windows version of awk (like gawk) and the program 7-zip to make the package
- Max version 6 at least
- Arduino IDE 1.8.1
- Teensyduino 1.35 (teensy support for the Arduino IDE and the teensy USB serial driver on Windows), with at least the i2c_t3 library. Note: Teensyduino already includes the Teensy serial driver on Windows

## Project organisation
- the folder "doc" contains the documentation
- the folder "firmware" contains the sources of the firmware to be uploaded in the teensy board
- the folder "libhedrot" contains the sources of the receiver library
- the folder "Max" contains the sources of the main receiver application, written in Max
- the folder "matlab" contains programs for matlab and octave
- the folder "command-line-demo" contains the sources of the command-line demonstration
- the folder "scripts" contains the scripts for building binaries and the distribution
- the empty folder "build" is where the distribution will be built
