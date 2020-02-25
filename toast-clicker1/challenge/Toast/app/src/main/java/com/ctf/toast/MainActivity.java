package com.ctf.toast;

import androidx.appcompat.app.AppCompatActivity;

import android.app.DownloadManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;
import android.util.Log;

import java.io.File;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.List;
import java.util.Random;

import dalvik.system.DexClassLoader;

public class MainActivity extends AppCompatActivity {
    private long downloadID;
    SharedPreferences preferences;
    int input[] = {67, 83, 68, 120, 62, 109, 95, 90, 92, 112, 85, 73, 99, 82, 53, 99, 101, 92, 80, 89, 81, 104};
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        final TextView toastCounter = (TextView) findViewById(R.id.clickCounter);
        final ImageView toastView = (ImageView) findViewById(R.id.toast_imageView);
        final Animation animShake = AnimationUtils.loadAnimation(this, R.anim.shake);
        registerReceiver(onDownloadComplete,new IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE));
        preferences = PreferenceManager.getDefaultSharedPreferences(this);
        String score = preferences.getString("score","0");
        toastCounter.setText(score);
        toastView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String newScore = incrementCounter(toastCounter.getText().toString());
                toastCounter.setText(newScore);
                Context context = getApplicationContext();
                List<String> toastTypes = Arrays.asList("Burnt Toast","Crispy Toast", "Yummy Toast", "Plain Toast", "Perfect Toast");
                Random rand = new Random();
                String randomToast = toastTypes.get(rand.nextInt(toastTypes.size()));
                int duration = Toast.LENGTH_SHORT;
                Toast toast = Toast.makeText(context, randomToast, duration);
                toast.show();
                toastView.startAnimation(animShake);
            }
        });
        final Button button = (Button) findViewById(R.id.downloadbutton);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                downloadFile();
            }
        });

    }
    @Override
    protected void onPause(){
        TextView toastCounter = (TextView) findViewById(R.id.clickCounter);
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
        SharedPreferences.Editor edit = preferences.edit();
        edit.putString("score", toastCounter.getText().toString());
        edit.apply();
        super.onPause();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unregisterReceiver(onDownloadComplete);
    }

    private BroadcastReceiver onDownloadComplete = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            long id = intent.getLongExtra(DownloadManager.EXTRA_DOWNLOAD_ID, -1);
            if (downloadID == id) {
                Toast.makeText(MainActivity.this, "Pro download completed", Toast.LENGTH_SHORT).show();
                Log.d("Download","Completed");
                loadClass();
            }
        }
    };
    private void downloadFile(){
        File file = new File(getExternalFilesDir(null),"bacon-final.dex");
        DownloadManager downloadmanager = (DownloadManager) getSystemService(Context.DOWNLOAD_SERVICE);
        Uri uri = Uri.parse("https://storage.googleapis.com/bsides-sf-ctf-2020-attachments/bacon-final.dex");
        DownloadManager.Request request = new DownloadManager.Request(uri);
        request.setTitle("Dex File");
        request.setDescription("Downloading update");
        request.setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED);
        request.setVisibleInDownloadsUi(false);
        request.setDestinationUri(Uri.fromFile(file));
        request.setAllowedOverRoaming(false);
        request.setAllowedOverMetered(false);
        Log.d("File path",Uri.fromFile(file).toString());
        downloadID = downloadmanager.enqueue(request);
    }
    private void loadClass(){
        File dexFile = new File(getExternalFilesDir(null),"bacon-final.dex");
        String className = "bacon.ToastDynamicFlag";
        String methodToInvoke = "printThirdFlag";
        DexClassLoader dLoader = new DexClassLoader(Uri.fromFile(dexFile).toString(),null,null,ClassLoader.getSystemClassLoader().getParent());
        try {
            Class<?> loadedClass = dLoader.loadClass(className);
            Object obj = loadedClass.newInstance();
            String flagPart1 = "ijiijiiijjjjjijijijiiijjijjjji";
            String flagPart2 = "jjjiiiiijjjijijijjijiijji";
            Method m = loadedClass.getMethod(methodToInvoke, String.class, String.class);
            String flag = (String) m.invoke(obj, flagPart1, flagPart2);

        } catch (ClassNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (InstantiationException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (NoSuchMethodException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    String incrementCounter(String strCounter){
        int counter = 0;
        if (strCounter == "") {
            return String.valueOf(counter);
        }
        else{
            counter = Integer.parseInt(strCounter);
            return String.valueOf(counter + 1);
        }
    }
    String printfirstFlag(){
        String output = "";
        for(int i = 0; i < input.length; i++ ){
            int t = input[i] + i;
            output += Character.toString((char) t);
        }
        return output;
    }
    String printSecondFlag(){
        String output = "";
        String keyPart1 = BuildConfig.KEY_PART1;
        String keyPart2 = getString(R.string.key_part2);
        String key = keyPart1 + keyPart2 + keyStringFromJNI();
        helper h = null;
        try {
            h = new helper(key);
            output = h.decrypt(encryptedStringFromJNI());

        } catch (Exception e) {
            e.printStackTrace();
        }
        return output;
    }

    public native String  keyStringFromJNI();
    public native String  encryptedStringFromJNI();
    static {
        System.loadLibrary("native-lib");
    }
}
