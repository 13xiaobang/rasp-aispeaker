LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	IAiSpeakerManagerService.cpp \
	AiSpeakerManagerService.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	liblog \
	libbinder \
	libaispeakercore

LOCAL_MULTILIB := both
LOCAL_MODULE:= libaispeakermgr

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../include

ifneq ($(filter $(TARGET_CHIP_SUB_TYPE), mstar_838 mstar_938),)
LOCAL_CFLAGS += -DCFLAGS_MSTAR_838_938
endif

#${TOP}/external/libcxx/include

include $(BUILD_SHARED_LIBRARY)
