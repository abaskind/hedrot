package edu.polytechnique.smartrot.Activities;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;

import edu.polytechnique.smartrot.R;

/**
 * MenuActivity is the parent of all activity of the app
 * which displays a menu.
 *
 * Copyright 2017 Ecole Polytechnique
 * @author Tarek Marcé
 * @version 1.0
 */
abstract class MenuActivity extends AppCompatActivity {
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater menuInflater = getMenuInflater();
        menuInflater.inflate(R.menu.menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item){
        Log.d("Menu", "Click action");
        switch (item.getItemId()){
            case R.id.action_settings :
                openSettings();
                return true;
            case R.id.action_about :
                Log.d("Menu", "Click about");
                openAbout();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    private void openAbout(){
        Intent intent = new Intent(this, AboutActivity.class);
        startActivity(intent);
    }

    private void openSettings(){
        Intent intent = new Intent(this, AdvancedSettings.class);
        startActivity(intent);
    }
}
