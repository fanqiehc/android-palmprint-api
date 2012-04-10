package org.openpk.palmapi;

import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.PictureCallback;
import android.util.Log;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;

public class CameraView implements SurfaceHolder.Callback{
    private Camera camera_ = null;
    private SurfaceHolder surfaceHolder_ = null;
    private SurfaceView	  surfaceView_;
   
    private Camera.Size preSize_;
    private boolean tackingPicture_ = false;
    private boolean inProcessing_ = false;

    private int targetWidth = 0;
    private int targetHeight = 0;

    public CameraView(SurfaceView sv, int width, int height){
        surfaceView_ = sv;
        targetWidth = width;
        targetHeight = height;

        surfaceHolder_ = surfaceView_.getHolder();
        surfaceHolder_.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
        surfaceHolder_.addCallback(this);        
    }

    public int PictureWidth() {
        return preSize_.width;
    }
    public int PictureHeight() {
        return preSize_.height;
    }

    public void DoPreview(PreviewCallback cb) {
        if ( camera_ == null)
            return;

        camera_.stopPreview();
        camera_.setPreviewCallback(cb);
        camera_.startPreview();
    }

    public void StopPreview(){
        camera_.setPreviewCallback(null);
        camera_.stopPreview();
    }

    private void setupCamera() {
        camera_ = Camera.open();
        preSize_ = camera_.new Size(0, 0);
        Camera.Parameters p = camera_.getParameters();        
        
        for(Camera.Size s : p.getSupportedPreviewSizes()) {
            preSize_ = s;            
            if ( s.width == targetWidth || s.height == targetHeight )  {              
                break;
            }
        }
        p.setPreviewSize(preSize_.width, preSize_.height);
        p.setPictureSize(preSize_.width, preSize_.height);
        camera_.setParameters(p);
        camera_.setDisplayOrientation(90);
        try {
            camera_.setPreviewDisplay(surfaceHolder_);
        } catch ( Exception ex) {
            ex.printStackTrace(); 
        }
        camera_.startPreview();    
    }  
    
    @Override
    public void surfaceChanged(SurfaceHolder sh, int format, int w, int h){
    }
    
	@Override
    public void surfaceCreated(SurfaceHolder sh){        
        setupCamera();        
    }
    
	@Override
    public void surfaceDestroyed(SurfaceHolder sh){
        camera_.stopPreview();
        camera_.release();
        camera_ = null;
    }
}
