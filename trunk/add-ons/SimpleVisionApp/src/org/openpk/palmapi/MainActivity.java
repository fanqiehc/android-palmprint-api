package org.openpk.palmapi;

import org.openpk.palmapi.*;

import android.app.Activity;
import android.os.Bundle;
import android.view.Window;
import android.view.WindowManager;
import android.view.SurfaceView;

public class MainActivity extends Activity
{
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
        CameraView cameraView = new CameraView(cameraSurface, 640, 480);        

    }

}
