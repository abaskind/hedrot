<h1>SmartRot</h1>

SmartRot is a free application for headtracking. Its main purpose is for binaural sound rendering (3D-Audio on headphones), especially compatible with the free binaural monitoring plug-in MyBino ( http://www.cmap.polytechnique.fr/xaudio/mybino/ ).

SmartRot is the Android version of Hedrot, its available on Google Play Store : https://play.google.com/store/apps/details?id=edu.polytechnique.smartrot&hl=fr

The application uses the three sensors of the phone to compute orientation angles (yaw pitch roll) and send them to the renderer through OSC protocol.
Just set your computer local IP address ( https://www.howtofindmyipaddress.com/ ) and you’re good to go !

Touch the screen to start sending orientation to MyBino and press a second time to center angles.
A long press on the screen stops the headtracker.
        <h1>About</h1>
        <p>
            This software was developped by the
            <a href="http://www.cmap.polytechnique.fr/xaudio/">X-Audio</a> team° : <br/>
            François Alouges - Professor<br/>
            Matthieu Aussal - Research Engineer<br/>
            Tarek Marcé - Android developer<br/>
            François Salmon - Sound Engineer<br/><br/>
            In collaboration with the CNSMDP° : <br/>
            Alexis Baskind - Sound Engineer<br/>
            Catherine de Boishéraud - Sound Engineer<br/>
            Jean-Marc Lyzwa - Sound Engineer<br/>
            Jean-Christophe Messonnier - Sound Engineer<br/><br/>
            Fundings° : <br/>
            CNRS - INSMI<br/>
            CNSMDP<br/>
            Ecole Polytechnique - CMAP<br/>
            <br/>
            ° in alphabetical order
        </p>
        
        <h1>License</h1>
        <p><a href="license.html">GNU General Public License</a><br/>
        </p>
        <p>
        SmartRot by <a href="http://www.cmap.polytechnique.fr/xaudio/">X-Audio</a>
            is the mobile version of <a href="https://abaskind.github.io/hedrot/">hedrot</a>.<br/>
        Part of code is derived from Sebastian Madgwick's open-source gradient descent angle
            estimation algorithm
            (<a href="http://x-io.co.uk/open-source-imu-and-ahrs-algorithms/">http://x-io.co.uk/open-source-imu-and-ahrs-algorithms/</a>).<br/>
        This programm use <a href="http://www.illposed.com/software/javaosc.html">Illposed's JavaOSC library</a>
            (<a href="JAVAOSC_LICENSE">License</a>)
        </p>
        <h1>OSC messages</h1>
        <p>
            Angles are send with three different OSC messages:
        </p>
            <ul>
                <li>/smartrot/yaw</li>
                <li>/smartrot/pitch</li>
                <li>/smartrot/roll</li>
            </ul>
