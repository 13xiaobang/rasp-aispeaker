#ifndef AI_SPEAKER_TYPES_H
#define AI_SPEAKER_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

enum SERVER_STATUS {
    STATUS_IDLE = 0,
    STATUS_POLLING,
    STATUS_RECEIVED,
};

enum SERVER_PCM_STATUS{
    STATUS_PCM_IDLE = 0,
    STATUS_PCM_WAKEUP,
    STATUS_PCM_POLLING,
    STATUS_PCM_POLLING_DONE,
};

struct ai_speaker_info
{
    pthread_mutex_t info_mutex;
    //unsigned int data;
    //unsigned int status;
    unsigned int pcm_status;
    char *pcm_data;
    unsigned int pcm_offset;
    unsigned int pcm_upload;
};

#define SERVER_PORT 5678
#define PCM_BUFFER_SIZE (256*1024) //max used buffer
#define PCM_UPLOAD_SIZE (6*1024)
#define PCM_START_TRANSFER "start_transfer"
#define PCM_STOP_TRANSFER "stop_transfer"
#define PCM_WAKE_UP "wake_up"
#define PCM_ASK_ALIVE "ask_alive"
#define PCM_ASK_BT_ADDR "ask_bt_addr"
#define PCM_IS_ALIVE "is_alive"
#define PCM_NOT_ALIVE "not_alive"

#define PCM_NO_BT_ADDR "no_bt_addr"

#ifdef __cplusplus
}
#endif

#endif