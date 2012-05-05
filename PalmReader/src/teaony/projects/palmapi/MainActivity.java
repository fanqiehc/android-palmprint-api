package teaonly.projects.palmapi;

import teaonly.projects.palmapi.*;

import java.io.IOException;
import java.io.File;
import java.io.FileOutputStream;
import java.lang.System;
import java.lang.Thread;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.ProgressDialog;
import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.PictureCallback;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.LayoutInflater;
import android.view.View;
import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;
import android.view.SurfaceView;
import android.util.Log;
import android.widget.Button;
import android.widget.TextView;
import android.webkit.WebView;

public class MainActivity extends Activity 
    implements View.OnTouchListener, CameraView.CameraReadyCallback, OverlayView.UpdateDoneCallback{
    private enum AppState{
        INITED, LABELING, PROCESSING, DISPLAY_SHOW,    
    }

    private CameraView cameraView_;
    private OverlayView overlayView_;
    private ProgressDialog processDialog = null;
    private AlertDialog helperDialog = null;
    private Button btnNext;
    private TextView tvMsg;

    private AppState state_ = AppState.INITED;

    private byte[] rawFrame_ = null; 
    private byte[] labelFrame_ = null; 
    private Bitmap resultBMP_;

    private boolean labelProcessing_ = false;
    private LabelThread labelThread_ = null;
    private ProcThread procThread_ = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        NativeAPI.LoadLibraries();

        requestWindowFeature(Window.FEATURE_NO_TITLE);
        Window win = getWindow();
        win.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);    
        win.setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN); 

        setContentView(R.layout.main);

    
        btnNext = (Button)findViewById(R.id.btn_next);
        tvMsg = (TextView)findViewById(R.id.tv_message);

        SurfaceView cameraSurface = (SurfaceView)findViewById(R.id.surface_camera);
        cameraView_ = new CameraView(cameraSurface, 640, 480, 640, 480);        
        cameraView_.setCameraReadyCallback(this);

        overlayView_ = (OverlayView)findViewById(R.id.surface_overlay);
        overlayView_.setOnTouchListener(this);
        overlayView_.setUpdateDoneCallback(this);
        
        new Handler(Looper.getMainLooper()).post(new Runnable(){
                public void run() { 
                    LayoutInflater inflater = LayoutInflater.from(MainActivity.this);
                    View helperView = inflater.inflate(R.layout.helper, null);
                    
                    WebView webview = (WebView)helperView.findViewById(R.id.webview);
                    webview.getSettings().setJavaScriptEnabled(true);
                    webview.setVerticalScrollBarEnabled(false);
                    webview.addJavascriptInterface(MainActivity.this, "Helper");
                    webview.loadUrl("file:///android_asset/helper.html");

                    AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);
                    builder.setView(helperView);
                    helperDialog = builder.create();
                    helperDialog.show();
                }   
           });     
    }
    
    @Override
    public void onCameraReady() {
        int wid = cameraView_.PreviewWidth();
        int hei = cameraView_.PreviewHeight();
        rawFrame_ = new byte[wid * hei + wid * hei / 2];
        labelFrame_ = new byte[wid*hei/2];
        resultBMP_ = Bitmap.createBitmap(overlayView_.getWidth(), overlayView_.getHeight(), Bitmap.Config.ARGB_8888);        
        
        NativeAPI.nativePrepare(wid, hei, 2);     

        state_ = AppState.LABELING;
        cameraView_.SetPreview( previewCb_ ); 
    }
 
    @Override
    public void onUpdateDone() {
        labelProcessing_ = false;     
    }

    @Override
    public void onDestroy(){
        super.onDestroy();
    }   

    @Override
    public void onStart(){
        super.onStart();
    }   

    @Override
    public void onResume(){
        super.onResume();
    }   
    
    @Override
    public void onPause(){    
        super.onPause();
    }  
    
    @Override
    public void onBackPressed() {
        super.onBackPressed();
    }

    @Override
    public boolean onTouch(View v, MotionEvent evt) {
        if ( state_ == AppState.LABELING ) {
            cameraView_.SetPreview(null);
            cameraView_.StopPreview();

            processDialog = ProgressDialog.show(this, "", "Processing...", true);  
            processDialog.show();

            waitCompleteLastLabeling();
            labelProcessing_ = false;
            state_ = AppState.PROCESSING;
            procThread_ = new ProcThread();
            procThread_.start();
        } else if ( state_ == AppState.DISPLAY_SHOW){
            tvMsg.setVisibility(View.GONE);
            btnNext.setVisibility(View.GONE);
            state_ = AppState.LABELING;
            cameraView_.SetPreview(previewCb_);
            cameraView_.StartPreview();
        }

        return false;
    } 

    public void hideHelper() {
        if (helperDialog != null) {
            new Handler(Looper.getMainLooper()).post(new Runnable(){
                public void run() { 
                    helperDialog.hide();
                    } 
                });                   
        }
    }

    private void waitCompleteLastLabeling() {
        if ( labelThread_ == null)
            return;

        if( labelThread_.isAlive() ){
            try {
                labelThread_.join();
                labelThread_ = null;
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    private PreviewCallback previewCb_ = new PreviewCallback() {
        public void onPreviewFrame(byte[] frame, Camera c) {
            if ( state_ != AppState.LABELING)
                return;
            
            if ( labelProcessing_ )
                return;

            labelProcessing_ = true; 
            int wid = cameraView_.PreviewWidth();
            int hei = cameraView_.PreviewHeight();             

            ByteBuffer bbuffer = ByteBuffer.wrap(frame); 
            bbuffer.get(rawFrame_, 0, wid * hei + wid * hei / 2);

            waitCompleteLastLabeling();
            labelThread_ = new LabelThread();
            labelThread_.start();
        }
    };

    private class LabelThread extends Thread{
        private int width = cameraView_.PreviewWidth();
        private int height = cameraView_.PreviewHeight();

        @Override
        public void run() {           
            resultBMP_.eraseColor(Color.TRANSPARENT);
            NativeAPI.nativeLabelPalm( rawFrame_, labelFrame_, resultBMP_ );
            overlayView_.DrawResult( resultBMP_ );
        }
    }

    private class ProcThread extends Thread {

        @Override
        public void run() {           
            resultBMP_.eraseColor(Color.TRANSPARENT);
            final int ret = NativeAPI.nativeReadingPalm(labelFrame_, rawFrame_, resultBMP_); 
            overlayView_.DrawResult( resultBMP_ );
            new Handler(Looper.getMainLooper()).post(new Runnable(){
                public void run() { 
                    if (processDialog != null) {
                        processDialog.dismiss();
                        processDialog = null;   
                        state_ = AppState.DISPLAY_SHOW;
                    } 
                    if ( ret > 0) {
                        tvMsg.setText("If lines is detected ok, press bellow button to see the result!\n If lines is not exacted with yours, touch screen to try again!");
                        btnNext.setVisibility(View.VISIBLE);         
                        tvMsg.setVisibility(View.VISIBLE);
                    } else {
                        tvMsg.setText("If lines is detected error, touch screen to try again!");
                        tvMsg.setVisibility(View.VISIBLE);
                    }
                }   
            });

        }
    }

}
