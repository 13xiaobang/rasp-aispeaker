LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libaispeakermgr_jni
LOCAL_SRC_FILES := jni/ai_speaker_AiSpeakerManager.cpp

LOCAL_SHARED_LIBRARIES := libaispeakermgr \
                        libcutils \
                        libutils \
                        liblog \
                        libbinder \
                        libnativehelper

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include \
                    $(TOP)/external/libcxx/include

LOCAL_MULTILIB := both
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
include $(BUILD_MULTI_PREBUILT)
include $(CLEAR_VAR)
LOCAL_JACK_ENABLED := disabled
LOCAL_MODULE := ai.speaker
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-java-files-under, java)
include $(BUILD_STATIC_JAVA_LIBRARY)
