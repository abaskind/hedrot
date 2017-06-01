package edu.polytechnique.smartrot.Activities;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.webkit.WebView;

import edu.polytechnique.smartrot.R;

/**
 * About View.
 * Basically a WebView using HTML file.
 *
 * Copyright 2017 Ecole Polytechnique
 * @author Tarek Marcé
 * @version 1.0
 */
public class AboutActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_about);
        WebView aboutWV = (WebView) findViewById(R.id.aboutWV);
        aboutWV.loadUrl("file:///android_asset/about.html");
    }
}
