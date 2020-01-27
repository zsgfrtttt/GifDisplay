package com.csz.gif;

import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.ImageView;

import java.io.File;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {

    private ImageView iv;
    GifLoader loader;

    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        iv = findViewById(R.id.iv);
    }

    public void loadGif(View view) {
        String path = Environment.getExternalStorageDirectory().getAbsolutePath()+ File.separator+"abc.gif";
        loader = GifLoader.create(path);
        loader.into(iv);
    }

    @Override
    protected void onDestroy() {
        if (loader != null){
            loader.release();
        }
        super.onDestroy();
    }
}
