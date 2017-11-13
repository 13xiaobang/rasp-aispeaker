#define LOG_TAG "ai_speaker_server"
#include <utils/Log.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libaispeakercore/ai_speaker_types.h>
#include <libaispeakercore/virtual_key.h>
#include <cutils/properties.h>

//if is cmd, return 1, else return 0
int process_pcm_cmd_and_data(struct ai_speaker_info* speaker_info, char* buffer, int
iDataNum, int client)
{
    if(speaker_info->pcm_status == STATUS_PCM_IDLE &&!strncmp(buffer, PCM_WAKE_UP, strlen(PCM_WAKE_UP)))
    {
        ALOGE("PCM_WAKE_UP got");
        speaker_info->pcm_status = STATUS_PCM_WAKEUP;
        speaker_info->pcm_offset = 0;
        speaker_info->pcm_upload = 0;
        send_vk_event(VK_WAKEUP_EVENT);
        buffer += strlen(PCM_WAKE_UP);
        iDataNum -= strlen(PCM_WAKE_UP);
        if(iDataNum == 0)
            return 1;
        else
        {
            ALOGI("we got merged wakeup !!!!");
        }
    }

    if(speaker_info->pcm_status == STATUS_PCM_WAKEUP && !strncmp(buffer, PCM_START_TRANSFER, strlen(PCM_START_TRANSFER)))
    {
        ALOGE("we got start transfer !!!!!!!");
        buffer += strlen(PCM_START_TRANSFER);
        iDataNum -= strlen(PCM_START_TRANSFER);
        speaker_info->pcm_status = STATUS_PCM_POLLING;
        if(iDataNum == 0)
            return 1;
        else
        {
            ALOGI("we got merged transfer start !!!!");
        }
    }

    if(speaker_info->pcm_status == STATUS_PCM_POLLING && !strncmp(buffer, PCM_STOP_TRANSFER, strlen(PCM_STOP_TRANSFER)))
    {
        ALOGI("PCM_STOP_TRANSFER got");
        speaker_info->pcm_status = STATUS_PCM_POLLING_DONE;
        return 1;
    }

    if(speaker_info->pcm_status == STATUS_PCM_IDLE && !strncmp(buffer, PCM_ASK_ALIVE, strlen(PCM_ASK_ALIVE)))
    {
        ALOGI("PCM_ASK_ALIVE got");
        char prop_name[PROP_VALUE_MAX];

        property_get("sys.boot_completed", prop_name, 0);
        if(!strcmp(prop_name, "1"))
        {
            send(client, PCM_IS_ALIVE, strlen(PCM_IS_ALIVE), 0); // should not
// add 1 because python can not detect the last "\0"
        }
        else
            send(client, PCM_NOT_ALIVE, strlen(PCM_NOT_ALIVE), 0);
        // if not alive, just ignore.
        return 1;
    }

    if(speaker_info->pcm_status == STATUS_PCM_IDLE && !strncmp(buffer, PCM_ASK_BT_ADDR, strlen(PCM_ASK_BT_ADDR)))
    {
        ALOGI("PCM_ASK_BT_ADDR got");
        //here should return just like bcm11:22:33:44:55:66  bt addr for wake up this device.
        send(client, PCM_NO_BT_ADDR, strlen(PCM_NO_BT_ADDR), 0);
        return 1;
    }

    if (speaker_info->pcm_status == STATUS_PCM_POLLING) // here is raw pcm data.
    {
        //check if pcm stop is in the last.
        char * key = buffer + iDataNum - strlen(PCM_STOP_TRANSFER);
        if(!strncmp(key, PCM_STOP_TRANSFER, strlen(PCM_STOP_TRANSFER)))
        {
            ALOGE("we got merged stop end!!!!!!!");
            speaker_info->pcm_status = STATUS_PCM_POLLING_DONE;
            iDataNum -= strlen(PCM_STOP_TRANSFER);
        }

        if(speaker_info->pcm_offset + iDataNum > PCM_BUFFER_SIZE)
        {
            ALOGE("We get too many pcm data, do not process and wait for stop!!!");
            return 1;
        }
        memcpy(speaker_info->pcm_data + speaker_info->pcm_offset, buffer,
iDataNum);

        speaker_info->pcm_offset += iDataNum;
        return 1;
    }
    else
    {
        ALOGE("we should not come here !!!!!!!!!!!!!!!!!!");
        return 1;
    }
}

void *receive_ai_speaker_thread_routine(void *arg)
{
    struct ai_speaker_info* speaker_info = (struct ai_speaker_info*)arg;
    int serverSocket;

    struct sockaddr_in server_addr;
    struct sockaddr_in clientAddr;
    int addr_len = sizeof(clientAddr);
    int client;
    //char buffer[2048];
    char buffer[6400];

    int iDataNum;
    int on=1;
    speaker_info->pcm_data = (char*)malloc(PCM_BUFFER_SIZE);
    if(speaker_info->pcm_data == NULL)
    {
        ALOGE("malloc pcm data buffer failed.");
        return NULL;
    }

    if((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        ALOGE("socket error");
        return NULL;
    }

    if((setsockopt(serverSocket,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
    {
        ALOGE("setsockopt failed");
        return NULL;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(serverSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        ALOGE("connect error");
        return NULL;
    }

    if(listen(serverSocket, 5) < 0)
    {
        ALOGE("listen error");
        return NULL;
    }

    while(1)
    {
        ALOGI("Listening on port: %d\n", SERVER_PORT);

        client = accept(serverSocket, (struct sockaddr*)&clientAddr, (socklen_t*)&addr_len);
        if(client < 0)
        {
            ALOGE("accept error");
            continue;
        }
        ALOGI("\nrecv client data...n");

        ALOGI("IP is %s\n", inet_ntoa(clientAddr.sin_addr));
        ALOGI("Port is %d\n", htons(clientAddr.sin_port));
        while(1)
        {
            iDataNum = recv(client, buffer, PCM_UPLOAD_SIZE, 0);
            ALOGE("iDataNum = recv = %d\n", iDataNum);
            if(iDataNum < 0)
            {
                ALOGE("recv info error");
                continue;
            }
            else if(iDataNum == 0)
            {
                ALOGE("ERROR, iDataNum == 0, break");
                break;
            }
            buffer[iDataNum] = '\0';
            pthread_mutex_lock(&speaker_info->info_mutex);
            if(process_pcm_cmd_and_data(speaker_info, buffer, iDataNum, client))
            {
                pthread_mutex_unlock(&speaker_info->info_mutex);
                continue;
            }
            pthread_mutex_unlock(&speaker_info->info_mutex);
        }
    }
    close(client);
    close(serverSocket);
    free(speaker_info->pcm_data);
    ALOGI("receive_ai_speaker_thread_routine closed!");
    return NULL;
}
