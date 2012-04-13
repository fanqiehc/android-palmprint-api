package teaonly.projects.palmapi;

import java.io.*; 
import java.net.*;
import android.util.Log;

class NativeAPI {
    public static native int nativePrepare(int preWid, int preHei, int procWid, int procHei);
    public static native int nativeLabelPalm(byte[] frame, int width, int height, Object bmp);

    public static void LoadLibraries() {
        System.loadLibrary("palmapi");
    }   
}
