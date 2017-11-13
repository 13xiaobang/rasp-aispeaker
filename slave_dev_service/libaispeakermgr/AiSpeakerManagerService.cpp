/***************************************************************************
 *
 * Copyright (c) 2016 Whaley Ltd. All Rights Reserved
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF Whaley Ltd.
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code.
 *
 ****************************************************************************/
#define LOG_TAG "ai_speaker_server"


#include <stdint.h>

#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/String16.h>
#include <unistd.h>


#include <binder/IServiceManager.h>

#include <libaispeakermgr/AiSpeakerManagerService.h>
#include <libaispeakercore/ai_speaker_types.h>
#include <libaispeakercore/virtual_key.h>
#define MAX_READ_SIZE 6400

extern struct ai_speaker_info g_ai_speaker_info;
namespace android {

void AiSpeakerManagerService::instantiate()
{
    defaultServiceManager()->addService(
            String16("aispeaker.manager"), new AiSpeakerManagerService());
}

AiSpeakerManagerService::AiSpeakerManagerService()
{
    ALOGV("AiSpeakerManagerService created");
}

AiSpeakerManagerService::~AiSpeakerManagerService()
{
    ALOGV("AiSpeakerManagerService destroyed");
}

#if 0
status_t AiSpeakerManagerService::setMode(uint32_t mode)
{
    ALOGI("setMode: %d", mode);
    g_ai_speaker_info.data = mode;
	if(mode == 0)
	{
		g_ai_speaker_info.pcm_status = STATUS_PCM_IDLE;
		g_ai_speaker_info.pcm_upload = 0;
	}
	else if(mode == 1) // audio pcm read done, send keyevent up.
		send_vk_event(VK_PCM_DONE_EVENT);
    return 0;
}

status_t AiSpeakerManagerService::getMode(uint32_t *mode)
{
    ALOGI("getMode called.");
    *mode = g_ai_speaker_info.data;
    return 0;
}

status_t AiSpeakerManagerService::pollEvent(uint32_t *event)
{
    ALOGI("pollEvent called.");
	pthread_mutex_lock(&g_ai_speaker_info.info_mutex);
	g_ai_speaker_info.status = STATUS_POLLING;
    pthread_mutex_unlock(&g_ai_speaker_info.info_mutex);

	while(1)
	{
		pthread_mutex_lock(&g_ai_speaker_info.info_mutex);
		if(g_ai_speaker_info.status == STATUS_RECEIVED)
		{
			*event = STATUS_RECEIVED;
			g_ai_speaker_info.status = STATUS_IDLE;
			pthread_mutex_unlock(&g_ai_speaker_info.info_mutex);
			break;
		}
		pthread_mutex_unlock(&g_ai_speaker_info.info_mutex);
		usleep(10*1000);
	}
    return 0;
}

status_t AiSpeakerManagerService::getStr(std::string& str)
{
    ALOGI("getStr called.");
    str = (char*)g_ai_speaker_info.str;
    return 0;
}

status_t AiSpeakerManagerService::getPcm(char **pcm, int32_t *size)
{
    ALOGI("getPcm called.");

	while(1)
	{
		pthread_mutex_lock(&g_ai_speaker_info.info_mutex);
		if(g_ai_speaker_info.pcm_status == STATUS_PCM_POLLING_DONE)
		{
			*pcm = g_ai_speaker_info.pcm_data;
			*size = g_ai_speaker_info.pcm_offset;
			break;
		}
		pthread_mutex_unlock(&g_ai_speaker_info.info_mutex);
		usleep(10*1000);
	}
	pthread_mutex_unlock(&g_ai_speaker_info.info_mutex);
	ALOGI("getPcm called done.");
    return 0;
}
#endif

status_t AiSpeakerManagerService::enquiry()
{
	ALOGI("AiSpeakerManagerService::enquiry called.");
	return MAX_READ_SIZE;
}

status_t AiSpeakerManagerService::open()
{
	ALOGI("AiSpeakerManagerService::open called.");
	return 0;
}

status_t AiSpeakerManagerService::close()
{
	ALOGI("AiSpeakerManagerService::close called.");
	return 0;
}

status_t AiSpeakerManagerService::read(char **pcm, int32_t *size)
{
	ALOGI("AiSpeakerManagerService::read called.");
	while(1)
	{
		pthread_mutex_lock(&g_ai_speaker_info.info_mutex);
		if(g_ai_speaker_info.pcm_status == STATUS_PCM_IDLE)
		{
			//*size = 0;
			*pcm = g_ai_speaker_info.pcm_data; //  feed no used buffer.
			pthread_mutex_unlock(&g_ai_speaker_info.info_mutex);
			return 0;
		}
		if(g_ai_speaker_info.pcm_status == STATUS_PCM_WAKEUP)
		{
			pthread_mutex_unlock(&g_ai_speaker_info.info_mutex);
			usleep(10*1000);
			continue;
		}

		if(g_ai_speaker_info.pcm_offset - g_ai_speaker_info.pcm_upload >= (unsigned int)(*size))
		{
			//do one upload.
			//*size = PCM_UPLOAD_SIZE;
			*pcm = g_ai_speaker_info.pcm_data + g_ai_speaker_info.pcm_upload;
			g_ai_speaker_info.pcm_upload += *size;
			pthread_mutex_unlock(&g_ai_speaker_info.info_mutex);
			return 0;
		}

		if(g_ai_speaker_info.pcm_status == STATUS_PCM_POLLING_DONE &&
				g_ai_speaker_info.pcm_offset - g_ai_speaker_info.pcm_upload < (unsigned int)(*size))
		{
			//*size = g_ai_speaker_info.pcm_offset - g_ai_speaker_info.pcm_upload;
			//*pcm = g_ai_speaker_info.pcm_data + g_ai_speaker_info.pcm_upload;
			*pcm = g_ai_speaker_info.pcm_data; //  feed no used buffer.

			g_ai_speaker_info.pcm_upload += *size;
			//set the up key.
			ALOGI("AiSpeakerManagerService:: VK_PCM_DONE_EVENT up.");
			g_ai_speaker_info.pcm_status = STATUS_PCM_IDLE;
			send_vk_event(VK_PCM_DONE_EVENT);
			pthread_mutex_unlock(&g_ai_speaker_info.info_mutex);
			//usleep(700*1000); // block for not upload 0
			ALOGI("AiSpeakerManagerService:: VK_PCM_DONE_EVENT up done.");
			return 0;
		}

		pthread_mutex_unlock(&g_ai_speaker_info.info_mutex);
		usleep(10*1000);
	}
	return 0;
}


}; // namespace android
