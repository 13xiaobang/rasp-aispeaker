SUBDIR_MAKEFILES := $(call all-named-subdir-makefiles,libaispeakercore libaispeakermgr framework test)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        main.cpp \

LOCAL_SHARED_LIBRARIES := \
        libcutils \
        libutils \
        liblog \
        libbinder \
        libaispeakercore \
        libaispeakermgr \
        libaispeakermgr_jni

LOCAL_CFLAGS := -DANDROID
LOCAL_MODULE:= aispeakerserver

LOCAL_C_INCLUDES += \
    $(TOP)/external/libcxx/include \
    $(LOCAL_PATH)/include \

include $(BUILD_EXECUTABLE)
include $(SUBDIR_MAKEFILES)
