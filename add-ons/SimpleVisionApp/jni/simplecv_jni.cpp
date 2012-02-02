#include <jni.h>
#include <android/log.h>

#define  LOG_TAG    "PALMAPI"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)  
#define  JNIDEFINE(fname) Java_org_openpk_palmapi_NativeAgent_##fname

extern "C" {
    JNIEXPORT void JNICALL JNIDEFINE(nativePrepare)(JNIEnv* env, jclass clz, jint preWid, jint preHei, jint picWid, jint picHei);
};

JNIEXPORT void JNICALL JNIDEFINE(nativePrepare)(JNIEnv* env, jclass clz, jint preWid, jint preHei, jint picWid, jint picHei) {
}

