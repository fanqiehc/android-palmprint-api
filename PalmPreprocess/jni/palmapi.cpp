#include <jni.h>
#include <android/log.h>
#include "palmapi.h"

#define  LOG_TAG    "TEAONLY"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)  
#define  JNIDEFINE(fname) Java_teaonly_projects_palmapi_NativeAPI_##fname

extern "C" {
    JNIEXPORT void JNICALL JNIDEFINE(nativePrepare)(JNIEnv* env, jclass clz, jint preWid, jint preHei, jint picWid, jint picHei);
    JNIEXPORT void JNICALL JNIDEFINE(nativeLabelPalm)(JNIEnv* env, jclass clz, jbyteArray src, jint wid, jint hei);
};

JNIEXPORT void JNICALL JNIDEFINE(nativePrepare)(JNIEnv* env, jclass clz, jint preWid, jint preHei, jint picWid, jint picHei) {
    PrepareLabelPalm(preWid, preHei);
}

JNIEXPORT void JNICALL JNIDEFINE(nativeLabelPalm)(JNIEnv* env, jclass clz, jbyteArray src, jint wid, jint hei) {
    jboolean b;    
	
    jbyte* framePtr = env->GetByteArrayElements(src,&b);

    LabelCentralArea((unsigned char *)framePtr, wid, hei);
    LabelPalmArea((unsigned char *)framePtr, wid, hei);
    
    env->ReleaseByteArrayElements(src, framePtr, 0);
}

