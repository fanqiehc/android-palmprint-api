#include <jni.h>
#include <android/log.h>
#include <android/bitmap.h>
#include "palmapi.h"

#define  LOG_TAG    "TEAONLY"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)  
#define  JNIDEFINE(fname) Java_teaonly_projects_palmapi_NativeAPI_##fname

extern "C" {
    JNIEXPORT void JNICALL JNIDEFINE(nativePrepare)(JNIEnv* env, jclass clz, jint preWid, jint preHei, jint picWid, jint picHei);
    JNIEXPORT void JNICALL JNIDEFINE(nativeLabelPalm)(JNIEnv* env, jclass clz, jbyteArray src, jint wid, jint hei, jobject bmp);
};

JNIEXPORT void JNICALL JNIDEFINE(nativePrepare)(JNIEnv* env, jclass clz, jint preWid, jint preHei, jint picWid, jint picHei) {
    PrepareLabelPalm(preWid, preHei);
}

JNIEXPORT void JNICALL JNIDEFINE(nativeLabelPalm)(JNIEnv* env, jclass clz, jbyteArray src, jint wid, jint hei, jobject bmp) {
    jboolean b;    
	
    jbyte* framePtr = env->GetByteArrayElements(src,&b);
    LabelCentralArea((unsigned char *)framePtr, wid, hei);
    LabelPalmArea((unsigned char *)framePtr, wid, hei);

    // convert result to bitmap
	AndroidBitmapInfo  info;
	unsigned int *pixels;

	if ((AndroidBitmap_getInfo(env, bmp, &info)) < 0) {  
    	goto release;
    } 
    if ((AndroidBitmap_lockPixels(env, bmp, (void **)&pixels)) < 0) { 
        goto release;
    }

    for(int i = 0; i < hei; i++) {
    	for ( int j = 0; j < wid; j++) {
            if ( framePtr[j+i*wid] == 1){
                
                int y = (int)(1.0 * info.height / wid * j);
                int x = (int)(1.0 * info.width / hei * (hei - i));
                
                for(int m = x-1; m < x+1; m++) 
                for(int n = y-1; n < y+1; n++) {
                    if ( m > 0 && n > 0 && m < (int)info.width && n < (int)info.height) {
    		            unsigned int* rgba = pixels + m + n*info.stride/4;
                        *rgba = 0xFF00FFFF;
                    }
                }
            }
        }	
    }
    
    AndroidBitmap_unlockPixels(env, bmp);
release:    
	env->ReleaseByteArrayElements(src, framePtr, 0);   
}

