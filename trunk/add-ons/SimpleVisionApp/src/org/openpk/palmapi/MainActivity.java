package org.openpk.palmapi;

import org.openpk.palmapi.*;

import android.app.Activity;
import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.PictureCallback;
import android.os.Bundle;
import android.view.View;
import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;
import android.view.SurfaceView;

public class MainActivity extends Activity implements View.OnTouchListener {
    private CameraView cameraView;
    private VisionOverlay overlayView;

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        Window win = getWindow();
        win.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);    
        win.setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN); 

        setContentView(R.layout.main);

        SurfaceView cameraSurface = (SurfaceView)findViewById(R.id.surface_camera);
        cameraView = new CameraView(cameraSurface, 640, 480);        

        overlayView = (VisionOverlay)findViewById(R.id.surface_overlay);
        overlayView.setOnTouchListener(this);

        Processing.LoadLibraries();
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
        cameraView.DoPreview( myPreviewCb );
        return true;
    } 

    private PreviewCallback myPreviewCb = new PreviewCallback() {
        public void onPreviewFrame(byte[] frame, Camera c) {
            int result = Processing.nativeTest(frame);
            //int result = frame[0];
            overlayView.DrawResult(result);
        }
    };

}
