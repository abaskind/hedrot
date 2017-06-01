package edu.polytechnique.smartrot.Activities;


import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Toast;

import edu.polytechnique.smartrot.R;

/**
 * Advanced settings activity.
 * Use SharedPreferences.
 *
 * Copyright 2017 Ecole Polytechnique
 * @link https://developer.android.com/guide/topics/ui/settings.html
 * @author Tarek Marcé
 * @version 1.0
 */
public class AdvancedSettings extends AppCompatActivity implements View.OnClickListener{


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
        SharedPreferences.Editor e = sp.edit();
        e.putString(MainActivity.UPDATE_TIME, "2");
        e.putString(MainActivity.LOWPASSFILTER_ACCEL,"0.01");
        e.putString(MainActivity.BETA_MAX, "2.5");
        e.putString(MainActivity.BETA_GAIN, "1.");
        e.putString(MainActivity.AXIS_REF, "0");
        e.putString(MainActivity.ROTATIONS_ORDER, "0");
        e.putString(MainActivity.ROTATIONS_DIRECTION, "0");
        e.apply();
        putFragment();
    }

    private void putFragment(){
        getFragmentManager().beginTransaction()
                .replace(android.R.id.content, new SettingsFragment())
                .commit();
    }

    public static class SettingsFragment extends PreferenceFragment {
        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // Load the preferences from XML resource
            addPreferencesFromResource(R.xml.preferences);
        }
    }

    @Override
    public void onClick(View v) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
        SharedPreferences.Editor e = sp.edit();
        e.putString(MainActivity.UPDATE_TIME, "2");
        e.putString(MainActivity.LOWPASSFILTER_ACCEL,"0.01");
        e.putString(MainActivity.BETA_MAX, "2.5");
        e.putString(MainActivity.BETA_GAIN, "1.");
        e.putString(MainActivity.AXIS_REF, "0");
        e.putString(MainActivity.ROTATIONS_ORDER, "0");
        e.putString(MainActivity.ROTATIONS_DIRECTION, "0");
        e.apply();
        putFragment();
        (Toast.makeText(this,getString(R.string.restore_default),Toast.LENGTH_LONG)).show();
    }
}
