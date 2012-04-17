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
import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.PictureCallback;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.view.View;
import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;
import android.view.SurfaceView;
import android.util.Log;

public class MainActivity extends Activity 
    implements View.OnTouchListener, CameraView.CameraReadyCallback, OverlayView.UpdateDoneCallback{
    private enum AppState{
        INITED, LABELING, PROCESSING, DISPLAY_SHOW,    
    }

    private CameraView cameraView_;
    private OverlayView overlayView_;

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

        SurfaceView cameraSurface = (SurfaceView)findViewById(R.id.surface_camera);
        cameraView_ = new CameraView(cameraSurface, 640, 480, 640, 480);        
        cameraView_.setCameraReadyCallback(this);

        overlayView_ = (OverlayView)findViewById(R.id.surface_overlay);
        overlayView_.setOnTouchListener(this);
        overlayView_.setUpdateDoneCallback(this);
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
    public boolean onTouch(View v, MotionEvent evt) {
        if ( state_ == AppState.LABELING ) {
            cameraView_.SetPreview(null);
            waitCompleteLastLabeling();
            labelProcessing_ = false;
            state_ = AppState.PROCESSING;
            cameraView_.StopPreview();
            procThread_ = new ProcThread();
            procThread_.start();
        }

        return false;
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
            NativeAPI.nativeEnhencePalm(labelFrame_, rawFrame_, resultBMP_); 
            overlayView_.DrawResult( resultBMP_ );
        }
    }

}
