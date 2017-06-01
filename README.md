SmartRot is a free application for headtracking. Its main purpose is for binaural sound rendering (3D-Audio on headphones), especially compatible with the free binaural monitoring plug-in MyBino ( http://www.cmap.polytechnique.fr/xaudio/mybino/ ).

SmartRot is the Android version of Hedrot, its available on Google Play Store : https://play.google.com/store/apps/details?id=edu.polytechnique.smartrot&hl=fr

The application uses the three sensors of the phone to compute orientation angles (yaw pitch roll) and send them to the renderer through OSC protocol.
Just set your computer local IP address ( https://www.howtofindmyipaddress.com/ ) and you’re good to go !

Touch the screen to start sending orientation to MyBino and press a second time to center angles.
A long press on the screen stops the headtracker.

OSC messages format :
/smartrot/yaw
/smartrot/pitch
/smartrot/roll
