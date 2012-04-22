package teaonly.projects.palmapi;

import java.io.*; 
import java.net.*;
import android.util.Log;

class NativeAPI {
    public static native int nativePrepare(int preWid, int preHei, int scale);
    public static native int nativeLabelPalm(byte[] srcFrame, byte[] dstFrame, Object bmp);
    public static native int nativeReadingPalm(byte[] map, byte[] frame, Object bmp);

    public static void LoadLibraries() {
        System.loadLibrary("palmapi");
    }   
}
