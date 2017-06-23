package edu.polytechnique.smartrot.Model;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;

import java.util.Timer;
import java.util.TimerTask;

import edu.polytechnique.smartrot.Activities.MainActivity;

/**
 * Headtracker class.
 * Wrap the AHRS algorithm and sensors management.
 *
 * Copyright 2017 Ecole Polytechnique
 * @author Tarek Marcé
 * @version 1.0
 */

public class Headtracker {
    /**
     * Context field allow us to get
     * the Views and SensorManager from
     * parent activity.
     */
    private final Context context;
    /**
     * Device's SensorManager provided by
     * Android to manipulate sensors.
     */
    private final SensorManager sensorManager;
    /**
     * Device's accelerometer sensor instance.
     */
    private final Sensor accel;
    /**
     * Device's gyroscope sensor instance.
     */
    private final Sensor gyro;
    /**
     * Device's magnetometer sensor instance.
     */
    private final Sensor magnet;
    /**
     * AHRS values.
     * values is the yaw pitch roll compute by AHRS algorithm.
     */
    private final float[] values;
    /**
     * Gyroscope raw values.
     * Provided by hardware sensor.
     */
    private final float[] gyroValues;
    /**
     * Accelerometer raw values.
     * Provided by hardware sensor.
     */
    private final float[] accelValues;
    /**
     * Magnetometer raw values.
     * Provided by hardware sensor.
     */
    private final float[] magnetValues;
    /**
     * Time of the last time
     * yaw-pitch-roll were compute.
     * Required by Madgwick AHRS algorithm.
     */
    private long lastUpdate;
    /**
     * MadgwickAHRS instance.
     * @see MadgwickAHRS
     */
    private final MadgwickAHRS madgwickAHRS;
    /**
     * Timer in charge of execute yaw pitch roll
     * computing at regular intervals.
     */
    private Timer timer;
    /**
     * Computation rate in milliseconds.
     */
    private int rate;
    /**
     * OSC class instance
     * @see OSC
     */
    private OSC osc;
    /**
     * Server ip address
     * (for OSC)
     * @see OSC
     */
    private String ip;
    /**
     * Network port number to use
     * (for OSC)
     * @see OSC
     */
    private int port;

    /**
     * Headtracker constructor
     * Initialize fields.
     * Instantiate SensorManager from application context.
     * Instantiate Sensors from SensorManager.
     * @param c Context of the activity
     */
    public Headtracker(Context c){
        this.context = c;
        this.sensorManager = (SensorManager) this.context.getSystemService(Context.SENSOR_SERVICE);
        this.accel = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        this.gyro = sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);
        this.magnet = sensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
        this.values = new float[3];
        this.accelValues = new float[3];
        this.gyroValues = new float[3];
        this.magnetValues = new float[3];
        this.rate = 10;
        this.madgwickAHRS = new MadgwickAHRS();
        this.lastUpdate = System.currentTimeMillis();
    }

    /**
     * SensorEventListener
     * In charge of record each sensor change
     * in gyroValues, accelValues and magnetValues arrays.
     * Also it will notify the View (thus the user) if magnetometer's
     * accuracy is not maximum.
     * Magnetometer's accuracy is provided by the system.
     * @see #gyroValues
     * @see #accelValues
     * @see #magnetValues
     */
    private final SensorEventListener sensorsListener = new SensorEventListener() {
        @Override
        public void onSensorChanged(SensorEvent event) {
            switch (event.sensor.getType()){
                case Sensor.TYPE_GYROSCOPE :
                    System.arraycopy(event.values, 0, gyroValues, 0, 3);
                    break;
                case Sensor.TYPE_ACCELEROMETER :
                    System.arraycopy(event.values, 0, accelValues, 0, 3);
                    break;
                case Sensor.TYPE_MAGNETIC_FIELD :
                    System.arraycopy(event.values, 0, magnetValues, 0, 3);
                    break;
            }
        }
        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {
            if(sensor.getType() == Sensor.TYPE_MAGNETIC_FIELD){
                switch (accuracy){
                    case SensorManager.SENSOR_STATUS_ACCURACY_HIGH:
                        break;
                    case SensorManager.SENSOR_STATUS_ACCURACY_MEDIUM:
                        ((MainActivity)context).notifyCalibrationMagneto();
                        break;
                    case SensorManager.SENSOR_STATUS_ACCURACY_LOW:
                        ((MainActivity)context).notifyCalibrationMagneto();
                        break;
                    default:break;
                }
            }
        }
    };

    /**
     * IP and Port number setter.
     * @param ip IP address to set.
     * @param port Port number to set.
     */
    public void setIpAndPort(String ip, int port){
        this.ip = ip;
        this.port = port;
    }

    /**
     * Start listening sensors,
     * start osc connexion
     * and schedule yaw pitch roll computation.
     * @see OSC#start()
     * @see SensorManager#registerListener(SensorEventListener, Sensor, int)
     * @see Timer#scheduleAtFixedRate(TimerTask, long, long)
     */
    public void start(){
        sensorManager.registerListener(sensorsListener, accel, SensorManager.SENSOR_DELAY_FASTEST);
        sensorManager.registerListener(sensorsListener, gyro, SensorManager.SENSOR_DELAY_FASTEST);
        sensorManager.registerListener(sensorsListener, magnet, SensorManager.SENSOR_DELAY_FASTEST);
        osc = new OSC(ip,port,values);
        osc.start();
        timer = new Timer();
        timer.scheduleAtFixedRate(new DoMadgwick(), 100, rate);
    }

    /**
     * Stop listening to sensors (save battery)
     * stop OSC connection
     * stop yaw-pitch-roll computation.
     * @see OSC#stopOSC()
     * @see Timer#cancel()
     */
    public void stop(){
        sensorManager.unregisterListener(sensorsListener);
        osc.stopOSC();
        timer.cancel();
    }

    /**
     * Define the zero from current
     * yaw pitch roll.
     */
    public void center(){
        madgwickAHRS.centerAngles();
    }
    /**
     * Yaw-pitch-roll getter.
     * @return A float array which contain yaw-pitch-roll
     * angles (in degrees).
     */
    public float[] getValues(){
        return this.values;
    }
    /**
     * Beta max setter.
     * Beta max is a value used by AHRS algorithm, it can be
     * change by user from settings.
     * @param b New value of BetaMax
     */
    public void setBetaMax(float b){
        madgwickAHRS.betaMax = b;
    }

    /**
     * Beta gain setter.
     * Beta gain is a value used by AHRS algorithm, it can be
     * change by user from settings.
     * @param b New value of Beta Gain.
     */
    public void setBetaGain(float b){
        madgwickAHRS.betaGain = b;
    }

    /**
     * Rate setter.
     * Rate is the time between 2 AHRS angles computations.
     * It can be change by user in settings.
     * @param i New rate value
     */
    public void setRate(int i){
        this.rate = i;
    }

    /**
     * Accelerometer Low-pass time constant setter.
     * It can be change by user in settings.
     * @param f New low-pass time constant value.
     */
    public void setAccLPconstant(float f){
        madgwickAHRS.setAccLPtimeConstant(f);
    }

    /**
     * Define orientation of references axis for AHRS.
     * It can be change by user in settings.
     * @param orientation An integer which can be 0 or 1 or 2.
     *                    0 : X=>right Y=>front Z=>down
     *                    1 : X=>right Y=>back Z=>above
     *                    2 : X=>down Y=>left Z=>above
     */
    public void setOrientation(int orientation) {
        madgwickAHRS.setOrientation(orientation);
    }
    /**
     * Define rotation order for AHRS.
     * Basically it invert Yaw and Roll values.
     * @param i 0 for YawPitchRoll 1 for RollPitchYaw
     */
    public void setRollPitchYaw(int i) {
        madgwickAHRS.setRotationOrder(i);
    }

    /**
     * Define rotations directions.
     * Basically change the sign of the angles.
     * @param i 0 for clockwise direction, 1 for counter clockwise direction.
     */
    public void setInvertRotationDirection(int i) {
        if (i == 0) madgwickAHRS.setInvertRotationDirection(false);
        if (i == 1) madgwickAHRS.setInvertRotationDirection(true);
    }

    /**
     * The Yaw-pitch-roll computation and OSC sending
     * TimerTask.
     * @see TimerTask
     */
    private class DoMadgwick extends TimerTask {
        @Override
        public void run() {
            long now = System.currentTimeMillis();
            madgwickAHRS.SamplePeriod = (now - lastUpdate) / 1000.0f;
            lastUpdate = now;
            madgwickAHRS.Update(gyroValues[0], gyroValues[1], gyroValues[2],
                        accelValues[0], accelValues[1], accelValues[2],
                        magnetValues[0], magnetValues[1], magnetValues[2]);
            values[0] = (float) madgwickAHRS.MadgYaw;
            values[1] = (float) madgwickAHRS.MadgPitch;
            values[2] = (float) madgwickAHRS.MadgRoll;
            //noinspection SynchronizeOnNonFinalField
            synchronized (osc){
                osc.notifyAll();
            }
        }
    }
}