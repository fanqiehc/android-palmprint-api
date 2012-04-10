#include <jni.h>
#include <android/log.h>

#define  LOG_TAG    "PALMAPI"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)  
#define  JNIDEFINE(fname) Java_org_openpk_palmapi_Processing_##fname

extern "C" {
    JNIEXPORT int JNICALL JNIDEFINE(nativeTest)(JNIEnv* env, jclass clz, jbyteArray src);
};

JNIEXPORT int JNICALL JNIDEFINE(nativeTest)(JNIEnv* env, jclass clz, jbyteArray src) {
    jboolean b;
    int ret;

    unsigned char * framePtr = 0;
    framePtr = (unsigned char *)(env->GetByteArrayElements(src,&b));
    ret = *framePtr + 10;
    env->ReleaseByteArrayElements(src, (jbyte *)framePtr, 0); 
    
    return ret;
}

