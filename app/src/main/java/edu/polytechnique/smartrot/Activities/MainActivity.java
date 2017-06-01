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
    // SharedPreferences keys
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

    @Override
    protected void onResume() {
        super.onResume();
        if(started){
            ht.start();
            startUpdateView();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if(started){
            ht.stop();
            stopUpdateView();
        }
    }

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
        }, 10, 10);
    }

    private void stopUpdateView(){
        textViewUpdater.cancel();
        textViewUpdater = null;
    }

    private void updateYPR(){
        float[] v = ht.getValues();
        yawTV.setText(String.format(Locale.getDefault(),"%.2f",v[0]));
        pitchTV.setText(String.format(Locale.getDefault(),"%.2f",v[1]));
        rollTV.setText(String.format(Locale.getDefault(),"%.2f",v[2]));
    }

    private void rotateImage(){
        float angle = ht.getValues()[0];
        imageButton.setRotation(-angle + 90);
    }

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

    private void soundAlert(){
        Ringtone r = RingtoneManager.getRingtone(MainActivity.this,
                RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION));
        r.play();
    }

    public void notifyCalibrationMagneto(){
        displayAlert(getString(R.string.lowprec_msg), getString(R.string.lowprec_title));
        soundAlert();
    }

    private void applySettings(){
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
        ht.setRate(Integer.valueOf(sp.getString(UPDATE_TIME, "2")));
        ht.setAccLPconstant(Float.valueOf(sp.getString(LOWPASSFILTER_ACCEL,"0.01")));
        ht.setBetaMax(Float.valueOf(sp.getString(BETA_MAX, "2.5")));
        ht.setBetaGain(Float.valueOf(sp.getString(BETA_GAIN, "1.")));
        ht.setOrientation(Integer.valueOf(sp.getString(AXIS_REF, "0")));
        ht.setRollPitchYaw(Integer.valueOf(sp.getString(ROTATIONS_ORDER, "0")));
        ht.setInvertRotationDirection(Integer.valueOf(sp.getString(ROTATIONS_DIRECTION, "0")));
    }
}
