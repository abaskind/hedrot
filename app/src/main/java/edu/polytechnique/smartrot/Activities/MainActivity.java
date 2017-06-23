package edu.polytechnique.smartrot.Activities;

import android.content.Intent;
import android.content.SharedPreferences;
import android.media.Ringtone;
import android.media.RingtoneManager;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v7.app.AlertDialog;
import android.text.method.LinkMovementMethod;
import android.view.View;
import android.view.WindowManager;
import android.widget.ImageButton;
import android.widget.TableLayout;
import android.widget.TextView;

import java.util.Locale;
import java.util.Timer;
import java.util.TimerTask;

import edu.polytechnique.smartrot.Model.Headtracker;
import edu.polytechnique.smartrot.R;

/**
 * MainActivity is the most important Activity of the app.
 * It allows the user to start and stop headtracking.
 * It also provides an advances settings button.
 *
 * Copyright 2017 Ecole Polytechnique
 * @author Tarek Marcé
 * @version 1.0
 */
public class MainActivity extends MenuActivity {
    // SharedPreferences constant keys
    static final String UPDATE_TIME = "pref_update_time";
    static final String LOWPASSFILTER_ACCEL = "pref_lowpassfilter_accel";
    static final String BETA_MAX = "pref_beta_max";
    static final String BETA_GAIN = "pref_beta_gain";
    static final String AXIS_REF = "pref_axis_ref";
    static final String ROTATIONS_ORDER = "pref_rotations_order";
    static final String ROTATIONS_DIRECTION = "pref_rotations_direction";

    private TableLayout tableTVLayout;
    private TextView yawTV;
    private TextView pitchTV;
    private TextView rollTV;
    private Timer textViewUpdater;
    private ImageButton imageButton;
    private boolean started = false;
    private Headtracker ht;
    private String ip;
    private int portNumber;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getViews();
        setListeners();
        Intent intent = getIntent();
        ip = intent.getStringExtra(WelcomeActivity.IP_ADDRESS);
        portNumber = intent.getIntExtra(WelcomeActivity.PORT_NUMBER, 2001);
        ht = new Headtracker(this);
        displayAlert(getString(R.string.usage), getString(R.string.usageTitle));
    }

    /**
     * Instanciate interractives view elements from XML
     */
    private void getViews(){
        tableTVLayout = (TableLayout) findViewById(R.id.tableTVLayout);
        tableTVLayout.setVisibility(View.GONE);
        yawTV = (TextView) findViewById(R.id.yawtv);
        pitchTV = (TextView) findViewById(R.id.pitchtv);
        rollTV = (TextView) findViewById(R.id.rolltv);
        imageButton = (ImageButton) findViewById(R.id.boutonLogo);
        TextView designedTV = (TextView) findViewById(R.id.bottom_line2);
        designedTV.setMovementMethod(LinkMovementMethod.getInstance());
    }

    private void setListeners(){
        imageButton.setOnClickListener(onClickListener);
        imageButton.setOnLongClickListener(onLongClickListener);
    }

    /**
     * On touch listener for the main button (big MyBino logo)
     */
    private final View.OnClickListener onClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            if(!started){
                applySettings();
                getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
                startUpdateView();
                tableTVLayout.setVisibility(View.VISIBLE);
                imageButton.setImageResource(R.drawable.ic_mybino_boussole_finale);
                started = true;
                ht.setIpAndPort(ip, portNumber);
                ht.start();
            }
            else{
                ht.center();
            }
        }
    };

    /**
     * On long touch listener for the main button (big MyBino logo)
     */
    private final View.OnLongClickListener onLongClickListener = new View.OnLongClickListener() {
        @Override
        public boolean onLongClick(View v) {
            if(started) {
                getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
                tableTVLayout.setVisibility(View.GONE);
                stopUpdateView();
                imageButton.setImageResource(R.drawable.ic_mybino_logo);
                started = false;
                ht.stop();
            }
            return true;
        }
    };

    /**
     * Relaunch the headtracker
     * if it was launched when app lost focus
     */
    @Override
    protected void onResume() {
        super.onResume();
        if(started){
            ht.start();
            startUpdateView();
        }
    }

    /**
     * Pause headtracker (sensor listening and OSC sending
     * when app lost focus.
     */
    @Override
    protected void onPause() {
        super.onPause();
        if(started){
            ht.stop();
            stopUpdateView();
        }
    }

    /**
     * Logo animation and angle display
     * Set to 30 fps (one image every 33ms)
     */
    private void startUpdateView(){
        textViewUpdater = new Timer();
        textViewUpdater.scheduleAtFixedRate(new TimerTask() {
            @Override
            public void run(){
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        updateYPR();
                        rotateImage();
                    }
                });
            }
        }, 10, 33);
    }

    /**
     * Stop animation
     */
    private void stopUpdateView(){
        textViewUpdater.cancel();
        textViewUpdater = null;
    }

    /**
     * Get angles from headtracker class and
     * refresh TextViews
     */
    private void updateYPR(){
        float[] v = ht.getValues();
        yawTV.setText(String.format(Locale.getDefault(),"%.2f",v[0]));
        pitchTV.setText(String.format(Locale.getDefault(),"%.2f",v[1]));
        rollTV.setText(String.format(Locale.getDefault(),"%.2f",v[2]));
    }

    /**
     * Rotate MyBino logo.
     */
    private void rotateImage(){
        float angle = ht.getValues()[0];
        int offset =
                (PreferenceManager.getDefaultSharedPreferences(this).getString(ROTATIONS_ORDER,"1"))
                .equals("0")?90:0;//turn the arrow inital direction in function of the rotation order
        imageButton.setRotation(-angle - offset);
    }

    /**
     * Pop a warning in front of the activity.
     * Mainly use for magnetometer calibration alert.
     * @param message Message displayed.
     * @param title Title of the window
     */
    private void displayAlert(String message, String title){
        final String m = message, t = title;
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AlertDialog.Builder b = new AlertDialog.Builder(MainActivity.this);
                b.setTitle(t);
                b.setMessage(m);
                b.setNeutralButton("OK", null);
                b.show();
                soundAlert();
            }
        });
    }

    /**
     * Perform a sound notification.
     */
    private void soundAlert(){
        Ringtone r = RingtoneManager.getRingtone(MainActivity.this,
                RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION));
        r.play();
    }

    /**
     * Notify user magnetometer's precision is low
     * Display a pop up and make sound
     */
    public void notifyCalibrationMagneto(){
        displayAlert(getString(R.string.lowprec_msg), getString(R.string.lowprec_title));
        soundAlert();
    }

    /**
     * Apply settings from preferences manager
     */
    private void applySettings(){
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
        ht.setRate(Integer.valueOf(sp.getString(UPDATE_TIME,
                getString(R.string.update_time_default))));
        ht.setAccLPconstant(Float.valueOf(sp.getString(LOWPASSFILTER_ACCEL,
                getString(R.string.lowpassfilter_accel_default))));
        ht.setBetaMax(Float.valueOf(sp.getString(BETA_MAX,
                getString(R.string.beta_max_default))));
        ht.setBetaGain(Float.valueOf(sp.getString(BETA_GAIN,
                getString(R.string.beta_gain_default))));
        ht.setOrientation(Integer.valueOf(sp.getString(AXIS_REF,
                getString(R.string.axis_ref_default))));
        ht.setRollPitchYaw(Integer.valueOf(sp.getString(ROTATIONS_ORDER,
                getString(R.string.rotations_order_default))));
        ht.setInvertRotationDirection(Integer.valueOf(sp.getString(ROTATIONS_DIRECTION,
                getString(R.string.rotations_direction_default))));
    }
}
