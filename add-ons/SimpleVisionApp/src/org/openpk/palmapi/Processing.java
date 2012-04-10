package org.openpk.palmapi;

import java.io.*; 
import java.net.*;
import android.util.Log;

class Processing {
    public static native int nativeTest(byte[] yuv);
    
    
    public static void LoadLibraries() {
        System.loadLibrary("simplecv");
    }   
}
