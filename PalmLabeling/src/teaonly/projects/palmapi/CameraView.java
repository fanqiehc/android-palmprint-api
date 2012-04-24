package teaonly.projects.palmapi;
import teaonly.projects.palmapi.*;

import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.PictureCallback;
import android.util.Log;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;

public class CameraView implements SurfaceHolder.Callback{
    public static interface CameraReadyCallback { 
        public void onCameraReady(); 
    }  

    private Camera camera_ = null;
    private SurfaceHolder surfaceHolder_ = null;
    private SurfaceView	  surfaceView_;
    CameraReadyCallback cameraReadyCb_ = null;
   
    private Camera.Size preSize_, procSize_;
    private boolean tackingPicture_ = false;
    private boolean inProcessing_ = false;

    private int previewWidth = 0;
    private int previewHeight = 0;

    private int processWidth = 0;
    private int processHeight = 0;


    public CameraView(SurfaceView sv, int width, int height, int picWid, int picHei){
        surfaceView_ = sv;
        previewWidth = width;
        previewHeight = height;
        processWidth = picWid;
        processHeight = picHei;

        surfaceHolder_ = surfaceView_.getHolder();
        surfaceHolder_.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
        surfaceHolder_.addCallback(this); 
    }

    public int PreviewWidth() {
        return preSize_.width;
    }

    public int PreviewHeight() {
        return preSize_.height;
    }

    public int PictureWidth() {
        return procSize_.width;
    }
    public int PictureHeight() {
        return procSize_.height;
    }

    public void setCameraReadyCallback(CameraReadyCallback cb) {
        cameraReadyCb_ = cb;
    }

    public void SetPreview(PreviewCallback cb) {
        if ( camera_ == null)
            return;

        camera_.setPreviewCallback(cb);
    }
    
    public void StartPreview(){
        if ( camera_ == null)
            return;
        camera_.startPreview();
    }
    
    public void StopPreview(){
        if ( camera_ == null)
            return;
        camera_.stopPreview();
    }

    public void TakePicture(PictureCallback cb) {
        camera_.takePicture(null, cb, null);
    }

    private void setupCamera() {
        camera_ = Camera.open();
        preSize_ = camera_.new Size(0, 0);
        Camera.Parameters p = camera_.getParameters();        
        
        for(Camera.Size s : p.getSupportedPreviewSizes()) {
            preSize_ = s;            
            if ( s.width == previewWidth || s.height == previewHeight )  {              
                break;
            }
        }
        p.setPreviewSize(preSize_.width, preSize_.height);
        
        for(Camera.Size s : p.getSupportedPreviewSizes()) {
            procSize_ = s;            
            if ( s.width == processWidth || s.height == processHeight )  {              
                break;
            }
        }
        p.setPictureSize(procSize_.width, procSize_.height);

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
        if ( cameraReadyCb_ != null)
            cameraReadyCb_.onCameraReady();
    }
    
	@Override
    public void surfaceDestroyed(SurfaceHolder sh){
        camera_.stopPreview();
        camera_.release();
        camera_ = null;
    }
}
