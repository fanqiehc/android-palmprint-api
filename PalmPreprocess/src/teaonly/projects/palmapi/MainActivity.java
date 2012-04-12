package teaonly.projects.palmapi;

import teaonly.projects.palmapi.*;

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

public class MainActivity extends Activity implements View.OnTouchListener, CameraView.CameraReadyCallback {
    private CameraView cameraView_;
    private OverlayView overlayView_;

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
        cameraView_ = new CameraView(cameraSurface, 640, 480);        
        cameraView_.setCameraReadyCallback(this);

        overlayView_ = (OverlayView)findViewById(R.id.surface_overlay);
        overlayView_.setOnTouchListener(this);
    }
    
    @Override
    public void onCameraReady() {
        NativeAPI.nativePrepare( cameraView_.PictureWidth(), cameraView_.PictureHeight() ); 
        cameraView_.DoPreview( previewCb_ ); 
    }

    @Override
    public void onCameraReady() {
        
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
        return super.onTouch(v, evnt);
    } 

    private PreviewCallback previewCb_ = new PreviewCallback() {
        public void onPreviewFrame(byte[] frame, Camera c) {
        }
    };

}
