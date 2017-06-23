package edu.polytechnique.smartrot.Activities;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v7.app.AlertDialog;
import android.text.method.LinkMovementMethod;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import edu.polytechnique.smartrot.R;

/**
 * First view of the app.
 * Main purpose is to ask user for the IP address of server
 * computer and port number to use.
 *
 * Copyright 2017 Ecole Polytechnique
 * @author Tarek Marcé
 * @version 1.0
 */
public class WelcomeActivity extends MenuActivity {
    // Extra messages constant keys
    public static final String IP_ADDRESS = "edu.polytechnique.smartrot.IP_ADDRESS";
    public static final String PORT_NUMBER = "edu.polytechnique.smartrot.PORT_NUMBER";

    private EditText ipET, portET;
    private TextView usageTV;
    private Button startBT;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_welcome);

        getViews();
        setListeners();
        getIpAndPort();
    }

    /**
     * Add the "about" and "settings" to the view
     * @param menu The in which we place our buttons
     * @return true (true means the menu will be displayed)
     */
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater menuInflater = getMenuInflater();
        menuInflater.inflate(R.menu.menu, menu);
        MenuItem settings = menu.findItem(R.id.action_settings);
        settings.setEnabled(false);
        settings.setVisible(false);
        return true;
    }

    /**
     * Instanciate view from XML
     */
    private void getViews(){
        startBT = (Button) findViewById(R.id.startBT);
        ipET = (EditText) findViewById(R.id.ipET);
        portET = (EditText) findViewById(R.id.portET);
        usageTV = (TextView) findViewById(R.id.usageTV);
        TextView designedTV = (TextView) findViewById(R.id.bottom_line1);
        designedTV.setMovementMethod(LinkMovementMethod.getInstance());
    }

    /**
     * Set listener on Start button, and ip/port number textinput.
     */
    private void setListeners(){
        startBT.setOnClickListener(onClickListener);
        ipET.setOnFocusChangeListener(onFocusChangeListener);
        portET.setOnFocusChangeListener(onFocusChangeListener);
    }

    /**
     * Start button listener.
     * Verify entry of the user.
     * Start the head-tracker activity (MainActivity)
     */
    private final View.OnClickListener onClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            Intent intent = new Intent(v.getContext(), MainActivity.class);
            String ip = ipET.getText().toString();
            int port;
            try {port = Integer.valueOf(portET.getText().toString());}
            catch (NumberFormatException nfe){port = -1;}
            try{
                controlEntries(ip,port);
                setIpAndPort(ip,port);
                intent.putExtra(IP_ADDRESS, ip);
                intent.putExtra(PORT_NUMBER, port);
                startActivity(intent);
            }
            catch (Exception e){printAlert(e.getMessage());}
        }
    };

    /**
     * Verify user's entries.
     * @param s The ip adress string.
     * @param i The port number integer.
     * @throws Exception in the case of the entry is invalid.
     */
    private void controlEntries(String s, int i) throws Exception{
        String error = "";
        if(s.isEmpty())error+=getString(R.string.IPEmpty);
        if(i == -1)error+=getString(R.string.InvalidPort);
        if(!error.isEmpty())throw new Exception(error);
    }

    /**
     * Print a pop-up alert.
     * Used for entries error (IP/port)
     * @param s The error message to print
     */
    private void printAlert(String s){
        AlertDialog.Builder adb = new AlertDialog.Builder(this);
        adb.setTitle(getString(R.string.AlertTitle));
        adb.setMessage(s);
        adb.setNeutralButton("OK", null);
        adb.show();
    }

    /**
     * On focus change listener, used for
     * print explanations about ip and port when the user
     * select on of those input.
     */
    private final View.OnFocusChangeListener onFocusChangeListener = new View.OnFocusChangeListener() {
        @Override
        public void onFocusChange(View v, boolean hasFocus) {
            if(hasFocus) {
                switch (v.getId()) {
                    case R.id.ipET:
                        usageTV.setText(getString(R.string.usageTVIP));
                        usageTV.setVisibility(View.VISIBLE);
                        break;
                    case R.id.portET:
                        usageTV.setText(getString(R.string.usageTVPort));
                        usageTV.setVisibility(View.VISIBLE);
                        break;
                    default:
                        break;
                }
            }
            else{
                usageTV.setVisibility(View.GONE);
            }
        }
    };

    /**
     * Get ip and port from sharred preference
     * (last ip and port the user use)
     */
    private void getIpAndPort(){
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
        if(sp.contains("pref_ip"))ipET.setText(sp.getString("pref_ip",""));
        if(sp.contains("pref_port"))portET.setText(sp.getString("pref_port",""));
    }

    /**
     * Set the ip and port in the SharedPreferences,
     * it allows to keep address and port from one use
     * to the other.
     * @param ip the ip to keep
     * @param port the port to keep
     */
    private void setIpAndPort(String ip, int port){
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
        SharedPreferences.Editor e =sp.edit();
        e.putString("pref_ip",ip);
        e.putString("pref_port",Integer.toString(port));
        e.apply();
    }
}
