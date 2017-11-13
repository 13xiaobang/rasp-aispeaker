LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	main_process.c \
    tcp_conn.c \
    virtual_key.c

LOCAL_SHARED_LIBRARIES := \
        libcutils \
        libutils \
        liblog

LOCAL_MULTILIB := both
LOCAL_CFLAGS := -DMSOS_TYPE_LINUX
LOCAL_MODULE:= libaispeakercore

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../include \
    external/iniparser \

include $(BUILD_SHARED_LIBRARY)
