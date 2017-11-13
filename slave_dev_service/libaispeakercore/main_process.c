#define LOG_TAG "ai_speaker_server"
#include <utils/Log.h>
#include <stdio.h>
#include <libaispeakercore/ai_speaker_types.h>
#include <libaispeakercore/tcp_conn.h>
#include <libaispeakercore/virtual_key.h>

pthread_t receive_ai_speaker_thread;

struct ai_speaker_info g_ai_speaker_info;


int start_receive_ai_speaker()
{
    int status = 0;
    status = pthread_create(&receive_ai_speaker_thread, NULL, receive_ai_speaker_thread_routine,
            (void*)(&g_ai_speaker_info));
    if (status != 0){
        ALOGE("Fail to create receive_ai_speaker_thread thread!");
        return 1;
    }
    return 0;
}


int ai_speaker_main_process()
{
    //setup_uinput_device();

    //memset(g_ai_speaker_info.str, 0, sizeof(g_ai_speaker_info.str));
    //strncat(g_ai_speaker_info.str,  "come here", strlen("come here"));
    pthread_mutex_init(&g_ai_speaker_info.info_mutex, NULL);
    //g_ai_speaker_info.status = STATUS_IDLE;
    g_ai_speaker_info.pcm_status = STATUS_PCM_IDLE;
    g_ai_speaker_info.pcm_offset = 0;
    g_ai_speaker_info.pcm_upload = 0;
    start_receive_ai_speaker();

    return 0;
}
