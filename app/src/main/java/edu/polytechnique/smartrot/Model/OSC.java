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
     * Constructor
     * Define IP address to connect and port to use.
     * @param ip
     * Address IP of the server (MyBino).
     * @param port
     * Port number to use.
     * @see #ipAddress
     * @see #portNumber
     */
    public OSC(String ip, int port){
        this.ipAddress = ip;
        this.portNumber = port;
    }
    /**
     * Run method, called from start() method of Thread class.
     * This will run the OSC Thread.
     * While the Thread is alive all yaw-pitch-roll values will
     * be send for each sendYawPitchRoll() method calls.
     * Stop the thread with stopOSC() method.
     * @see #sendYawPitchRoll(float[])
     * @see #stopOSC()
     * @see Thread
     */
    @Override
    public void run(){
        super.run();
        try {
            connect();
            //noinspection StatementWithEmptyBody
            while(this.keepGoing){
            }
        }catch (Exception e){
            interrupt();
        }
    }
    /**
     * Try to connect to the server through OSC.
     * IP address and port number uses are the ones give
     * as class constructor arguments.
     * @throws Exception
     * If connection unsuccessful throws Exception
     * @see #ipAddress
     * @see #portNumber
     */
    private void connect() throws Exception{
        portOut = new OSCPortOut(InetAddress.getByName(this.ipAddress), portNumber);
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
     * @param ypr a size 3 float array which contain yaw-pitch-roll values.
     */
    void sendYawPitchRoll(float[] ypr){
        send(new OSCMessage("/smartrot/yaw", Collections.singleton((Object)ypr[0])));
        send(new OSCMessage("/smartrot/pitch", Collections.singleton((Object)ypr[1])));
        send(new OSCMessage("/smartrot/roll", Collections.singleton((Object)ypr[2])));
    }
    /**
     * Stop the OSC thread
     */
    void stopOSC(){
        this.keepGoing = false;
    }
}
