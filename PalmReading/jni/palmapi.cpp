#include <jni.h>
#include <android/log.h>
#include <android/bitmap.h>
#include "palmapi.h"

#define  LOG_TAG    "TEAONLY"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)  
#define  JNIDEFINE(fname) Java_teaonly_projects_palmapi_NativeAPI_##fname


static int picWid, picHei, labelScale;

extern "C" {
    JNIEXPORT void JNICALL JNIDEFINE(nativePrepare)(JNIEnv* env, jclass clz, jint picWid, jint picHei, jint scale);
    JNIEXPORT void JNICALL JNIDEFINE(nativeLabelPalm)(JNIEnv* env, jclass clz, jbyteArray src, jbyteArray dst, jobject bmp);
    JNIEXPORT void JNICALL JNIDEFINE(nativeReadingPalm)(JNIEnv* env, jclass clz, jbyteArray map, jbyteArray frame, jobject bmp);
};

JNIEXPORT void JNICALL JNIDEFINE(nativePrepare)(JNIEnv* env, jclass clz, jint wid, jint hei, jint scale) {
    picWid = wid;
    picHei = hei;
    labelScale = scale;
    PrepareLabelPalm(picWid / scale, picHei / scale);
    PrepareEnhence(picWid, picHei);
    PrepareMarkLines(picWid, picHei);
}

JNIEXPORT void JNICALL JNIDEFINE(nativeLabelPalm)(JNIEnv* env, jclass clz, jbyteArray src, jbyteArray dst, jobject bmp) {
    jboolean b;    

    jbyte* framePtr = env->GetByteArrayElements(src, &b);
    jbyte* destPtr = env->GetByteArrayElements(dst, &b);

    LabelCentralArea((unsigned char *)framePtr, picWid, picHei, labelScale);
    LabelPalmArea((unsigned char *)destPtr);

    // convert result to bitmap
	AndroidBitmapInfo  info;
	unsigned int *pixels;
    int wid = picWid / labelScale;
    int hei = picHei / labelScale;
     // marker the central area.
    int ltx = wid*2/5;
    int lty = hei/4;
    int rbx = wid*2/5 + wid*2/5;
    int rby = hei/4 + hei/2;

	if ((AndroidBitmap_getInfo(env, bmp, &info)) < 0) {  
    	goto release;
    } 
    if ((AndroidBitmap_lockPixels(env, bmp, (void **)&pixels)) < 0) { 
        goto release;
    }
 
    for(int j = ltx; j <= rbx; j++) {
        int i = lty;
        int y = (int)(1.0 * info.height / wid * j);
        int x = (int)(1.0 * info.width / hei * (hei - i));
        unsigned int* rgba = pixels + x + y*info.stride/4;
        *rgba = 0xFFFFFFFF;

        i = rby;
        y = (int)(1.0 * info.height / wid * j);
        x = (int)(1.0 * info.width / hei * (hei - i));
        rgba = pixels + x + y*info.stride/4;
        *rgba = 0xFFFFFFFF;
    }
    for(int i = lty; i <= rby; i++) {
        int j = ltx;
        int y = (int)(1.0 * info.height / wid * j);
        int x = (int)(1.0 * info.width / hei * (hei - i));
        unsigned int* rgba = pixels + x + y*info.stride/4;
        *rgba = 0xFFFFFFFF;

        j = rbx;
        y = (int)(1.0 * info.height / wid * j);
        x = (int)(1.0 * info.width / hei * (hei - i));
        rgba = pixels + x + y*info.stride/4;
        *rgba = 0xFFFFFFFF;
    }

    for(int i = 0; i < hei; i++) {
    	for ( int j = 0; j < wid; j++) {
            if ( destPtr[j+i*wid] == 1){
                
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
            
            if ( destPtr[j+i*wid] > 0)
                destPtr[j+i*wid] = 1;
            else 
                destPtr[j+i*wid] = 0;            
        }	
    }
    
   AndroidBitmap_unlockPixels(env, bmp);
release:    
	env->ReleaseByteArrayElements(src, framePtr, 0);   
	env->ReleaseByteArrayElements(dst, destPtr, 0);   

}

JNIEXPORT void JNICALL JNIDEFINE(nativeReadingPalm)(JNIEnv* env, jclass clz, jbyteArray map, jbyteArray frame, jobject bmp) {
     jboolean b;    

    jbyte* mapPtr = env->GetByteArrayElements(map, &b);
    jbyte* framePtr = env->GetByteArrayElements(frame, &b);

    EnhencePalm((unsigned char *)mapPtr, (unsigned char *)framePtr, labelScale);
    MarkLines((unsigned char *)framePtr);

    // convert result to bitmap
	AndroidBitmapInfo  info;
	unsigned int *pixels;
 
	if ((AndroidBitmap_getInfo(env, bmp, &info)) < 0) {  
    	goto release;
    } 
    if ((AndroidBitmap_lockPixels(env, bmp, (void **)&pixels)) < 0) { 
        goto release;
    }
    
    for (int i = 0; i < (int)info.height; i++) {
        for (int j = 0; j < (int)info.width; j++) {
            int x = (int)(1.0 * i / info.height * picWid);
            int y = (int)(1.0 * (info.width - 1 - j) / info.width * picHei);
            unsigned char g = framePtr[x+y*picWid];
            unsigned int* rgba = pixels + j + i*info.stride/4;
            
            if ( g == 1) {
                *rgba = 0xFFFF0000;
            } else if (g==2) {
                *rgba = 0xFF00FF00;
            } else if (g==3) {
                *rgba = 0xFF0000FF;
            }
        }
    }
  
    AndroidBitmap_unlockPixels(env, bmp);

release:    
	env->ReleaseByteArrayElements(frame, framePtr, 0);   
	env->ReleaseByteArrayElements(map, mapPtr, 0);   
}
