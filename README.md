## SmartRot

SmartRot is a free application for headtracking. Its main purpose is for binaural sound rendering (3D-Audio on headphones), especially compatible with the free binaural monitoring plug-in MyBino ( http://www.cmap.polytechnique.fr/xaudio/mybino/ ).

SmartRot is the Android version of hedrot, its available on Google Play Store : https://play.google.com/store/apps/details?id=edu.polytechnique.smartrot&hl=fr

The application uses the three sensors of the phone to compute orientation angles (yaw pitch roll) and send them to the renderer through OSC protocol.
Just set your computer local IP address ( https://www.howtofindmyipaddress.com/ ) and you’re good to go !

Touch the screen to start sending orientation to MyBino and press a second time to center angles.
A long press on the screen stops the headtracker.

## About

This software was developped by the X-Audio team\* :
François Alouges - Professor
Matthieu Aussal - Research Engineer
Tarek Marcé - Android developer
François Salmon - Sound Engineer

In collaboration with the CNSMDP\* :
Alexis Baskind - Sound Engineer
Catherine de Boishéraud - Sound Engineer
Jean-Marc Lyzwa - Sound Engineer
Jean-Christophe Messonnier - Sound Engineer

Fundings\* :
CNRS - INSMI
CNSMDP
Ecole Polytechnique - CMAP

\* in alphabetical order

## License

GNU General Public License


SmartRot by X-Audio is the mobile version of hedrot.
Part of code is derived from Sebastian Madgwick's open-source gradient descent angle estimation algorithm (http://x-io.co.uk/open-source-imu-and-ahrs-algorithms/).
This programm use Illposed's JavaOSC library (License)
OSC messages

Angles are send with three different OSC messages:
/smartrot/yaw
/smartrot/pitch
/smartrot/roll
