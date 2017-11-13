#include <jni.h>

#ifndef WHALEY_TV_AI_SPEAKER_AISPEAKERMANAGER
#define WHALEY_TV_AI_SPEAKER_AISPEAKERMANAGER
#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved);

static jint native_setMode(JNIEnv *env, jobject thiz,jint mode);
static jint native_getMode(JNIEnv *env, jobject thiz);
static jint native_pollEvent(JNIEnv *env, jobject thiz);

static jstring native_getStr(JNIEnv *env, jobject thiz);
static jbyteArray native_getPcm(JNIEnv *env, jobject);

static jint native_enquiry(JNIEnv *env, jobject thiz);
static jint native_open(JNIEnv *env, jobject thiz);
static jint native_close(JNIEnv *env, jobject thiz);
//static jbyteArray native_read(JNIEnv *env, jobject);
static jint native_read(JNIEnv *env, jobject, jbyteArray javaAudioData, jint
sizeInBytes);


static JNINativeMethod ai_speaker_manager_method_table[] = {
/*
    {"setMode", "(I)I", (void*)native_setMode},
    {"getMode", "()I", (void*)native_getMode},
    {"pollEvent", "()I", (void*)native_pollEvent},
    {"getStr", "()Ljava/lang/String;", (void*)native_getStr},
    {"getPcm", "()[B", (void*)native_getPcm},
*/
//official interfaces:
    {"enquiry", "()I", (void*)native_enquiry},
    {"open", "()I", (void*)native_open},
    {"close", "()I", (void*)native_close},
    {"read", "([BI)I", (void*)native_read},
};


#ifdef __cplusplus
}
#endif
#endif
