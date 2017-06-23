package edu.polytechnique.smartrot.Model;

import com.illposed.osc.OSCMessage;
import com.illposed.osc.OSCPacket;
import com.illposed.osc.OSCPortOut;

import java.net.InetAddress;
import java.util.Collections;

/**
 * OSC Class
 * Handle network connections and OSC protocol.
 * Use Illposed's JavaOSC.
 *
 * Copyright 2017 Ecole Polytechnique
 * @see java.lang.Thread
 * @see java.lang.Runnable
 * @link http://www.illposed.com/software/javaoscdoc/
 * @author Tarek Marcé
 * @version 1.1
 */

class OSC extends Thread {
    /**
     * IP address of the server.
     * (IPv4 format)
     * @see String
     */
    private final String ipAddress;
    /**
     * Port number to use.
     */
    private final int portNumber;
    /**
     * OSC port out field.
     * @see OSCPortOut
     */
    private OSCPortOut portOut;
    /**
     * Thread activity field.
     * Thread stay alive while keepGoing is true.
     */
    private boolean keepGoing = true;
    /**
     * The yaw pitch roll values to send.
     * A simple copy of the "values" array in headtracker
     * class.
     */
    @SuppressWarnings("CanBeFinal")
    private float[] ypr;

    /**
     * Constructor
     * Define IP address to connect and port to use.
     * @param ip
     * Address IP of the server (MyBino).
     * @param port
     * Port number to use.
     * @param values
     * Yaw Pitch Roll values array,
     * transmit by headtracker class.
     * @see #ipAddress
     * @see #portNumber
     */
    public OSC(String ip, int port, float[] values){
        this.ipAddress = ip;
        this.portNumber = port;
        this.ypr = values;
    }
    /**
     * Run method, called from start() method of Thread class.
     * This will run the OSC Thread.
     * The Thread sleep between sendings.
     * When the thread is woke up by the notify() or notifyAll()
     * method the Thread send values and back to sleep.
     * Stop the thread with stopOSC() method.
     * @see #sendYawPitchRoll()
     * @see #stopOSC()
     * @see Thread
     */
    @Override
    public void run(){
        super.run();
        connect();
        while (keepGoing){
            sendYawPitchRoll();
            try {
                this.wait();
            }catch (Exception ignored){}
        }
    }
    /**
     * Try to connect to the server through OSC.
     * IP address and port number uses are the ones give
     * as class constructor arguments.
     * @see #ipAddress
     * @see #portNumber
     */
    private void connect(){
        try{
            portOut = new OSCPortOut(InetAddress.getByName(this.ipAddress), portNumber);
        }catch(Exception ignored){}
    }
    /**
     * Send a packet.
     * @param p the packet to send
     */
    private void send(OSCPacket p){
        try{
            portOut.send(p);
        }
        catch (Exception ignored){
        }
    }
    /**
     * Send yaw-pitch-roll values.
     * Angles are sends in degrees with 3 consecutive messages:
     * /smartrot/yaw
     * /smartrot/pitch
     * /smartrot/roll
     */
    private void sendYawPitchRoll(){
        send(new OSCMessage("/smartrot/yaw", Collections.singleton((Object) ypr[0])));
        send(new OSCMessage("/smartrot/pitch", Collections.singleton((Object) ypr[1])));
        send(new OSCMessage("/smartrot/roll", Collections.singleton((Object) ypr[2])));
    }
    /**
     * Stop the OSC thread
     */
    void stopOSC(){
        this.keepGoing = false;
    }
}
