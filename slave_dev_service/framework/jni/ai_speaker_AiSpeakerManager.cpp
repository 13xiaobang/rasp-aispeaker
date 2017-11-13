#define LOG_TAG "ai_speaker_server"

#include "jni.h"
#include "JNIHelp.h"

#include <utils/misc.h>
#include <utils/Log.h>

#include <stdio.h>
#include <utils/threads.h>
#include <unistd.h>
#include <sys/types.h>
#include <binder/IServiceManager.h>
#include "ai_speaker_AiSpeakerManager.h"
using namespace android;

#include <libaispeakermgr/AiSpeakerManagerService.h>
static unsigned char* g_read_buf;

const sp<IAiSpeakerManagerService> getAiSpeakerManagerService()
{
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder = sm->getService(String16("aispeaker.manager"));

    return interface_cast<IAiSpeakerManagerService>(binder);
}
/*
static jint native_setMode(JNIEnv *, jobject,jint mode)
{
    return getAiSpeakerManagerService()->setMode(mode);
}

static jint native_getMode(JNIEnv *, jobject)
{
    jint ret;
    jint status =  getAiSpeakerManagerService()->getMode((unsigned int*)&ret);
    if(status == NO_ERROR)
        return ret;
    return status;
}

static jint native_pollEvent(JNIEnv *, jobject)
{
    jint ret;
    jint status =  getAiSpeakerManagerService()->pollEvent((unsigned int*)&ret);
    if(status == NO_ERROR)
        return ret;
    return status;
}


static jstring native_getStr(JNIEnv *env, jobject)
{
	std::string buf;

    jint status =  getAiSpeakerManagerService()->getStr(buf);
    if(status == NO_ERROR)
    {
        return env->NewStringUTF(buf.c_str());
    }
    return NULL;
}



static jbyteArray native_getPcm(JNIEnv *env, jobject)
{
	char *buf = (char *)malloc(PCM_BUFFER_SIZE);
	int size;
	jbyteArray jr;
    jint status =  getAiSpeakerManagerService()->getPcm(&buf, &size);
    if(status == NO_ERROR)
    {
        jr=env->NewByteArray(size);
	    env->SetByteArrayRegion(jr, 0, size, (jbyte*)buf);
	    free(buf);
        return jr;
    }
	free(buf);
	jr=env->NewByteArray(0);
	env->SetByteArrayRegion(jr, 0, 0, NULL);
    return jr;
}
*/

#define PCM_BUFFER_SIZE (256*1024) //max used buffer
#define PCM_READ_SIZE (6*1024)

static jint native_enquiry(JNIEnv *, jobject)
{
    jint status =  getAiSpeakerManagerService()->enquiry();
    return status;
}

static jint native_open(JNIEnv *, jobject)
{
    jint status =  getAiSpeakerManagerService()->open();
    return status;
}

static jint native_close(JNIEnv *, jobject)
{
    jint status =  getAiSpeakerManagerService()->close();
    return status;
}

static jint native_read(JNIEnv *env, jobject, jbyteArray javaAudioData, jint sizeInBytes)
{
	//int size;
	char *temp;
	jbyte* recordBuff = (jbyte*)env->GetByteArrayElements(javaAudioData, NULL);

	if (recordBuff == NULL || sizeInBytes < 0)
	{
        ALOGE("Error retrieving destination for recorded audio data, can't record");
        return 0;
	}
	temp = (char*)recordBuff;
    jint status =  getAiSpeakerManagerService()->read(&temp, &sizeInBytes);
    if(status == NO_ERROR)
    {
        env->ReleaseByteArrayElements(javaAudioData, recordBuff, 0);
        return sizeInBytes;
    }
	env->ReleaseByteArrayElements(javaAudioData, recordBuff, 0);
    return 0;
}


#define JNIREG_CLASS "ai/speaker/AiSpeakerManager"

/*
* Register several native methods for one class.
*/
static int registerNativeMethods(JNIEnv* env, const char* className,
        JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;
    clazz = (env)->FindClass(className);
    if (clazz == NULL) {
        return JNI_FALSE;
    }

    if ((env)->RegisterNatives( clazz, gMethods, numMethods) < 0) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}


/*
* Register native methods for all classes we know about.
*/
static int registerNatives(JNIEnv* env)
{
    if (!registerNativeMethods(env, JNIREG_CLASS, ai_speaker_manager_method_table,
                                 sizeof(ai_speaker_manager_method_table) / sizeof(ai_speaker_manager_method_table[0])))
        return JNI_FALSE;

    return JNI_TRUE;
}

/*
* Set some test stuff up.
*
* Returns the JNI version on success, -1 on failure.
*/
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void*)
{
    JNIEnv* env = NULL;
    jint result = -1;
    ALOGD("JNI_OnLoad begin");
	g_read_buf = (unsigned char *)malloc(PCM_READ_SIZE);
	if(!g_read_buf)
	{
		ALOGE("JNI_OnLoad : malloc g_buf failed!");
		return -1;
	}
    if ((vm)->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    assert(env != NULL);
    if (!registerNatives(env)) {
        return -1;
    }
    /* success -- return valid version number */
    result = JNI_VERSION_1_6;
	ALOGD("JNI_OnLoad successfully");
    return result;
}

