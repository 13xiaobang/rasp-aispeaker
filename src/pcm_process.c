#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>  
#include <sys/types.h>    
#include <sys/socket.h>    
#include <sys/un.h> 

#include "../../include/msp_cmn.h"
#include "../../include/qivw.h"
#include "../../include/msp_errors.h"
#include "wb_vad.h"
#include "signal_processing_library.h"
#include "noise_suppression_x.h"
#include "noise_suppression.h"
#include "gain_control.h"


#define IVW_AUDIO_FILE_NAME "audio/1.pcm"
#define LIXIAN_FRAME_LEN	640 //16k采样率的16bit音频，一帧的大小为640B, 时长20ms
#define CAN_SERVICE "./CAN_SERVICE"
int accept_fd;
int socket_fd;

static int g_status = 0;//# 0: idle, 1: wake up detect,2: vad stop detect, will back to 0. 3: abnormal wakeup, will back to 0

void sleep_ms(int ms)
{
    usleep(ms * 1000);
}


// return 0, 1, 2 with correct answer, 3 if abnormal answer, 4 if fake wakeup.
int is_real_wakeup(char* result)
{
    //result = {"id":0,"score":-16,"eos":2330,"bos":390,"sst":"wakeup"}
    char *res[] = {"你好音响","搜索设备","和我说话"};
    char* pos = strstr(result, "\"score\":");
    if(pos)
    {
        pos += 8;
        if(pos[0] == '-')
            return 4;
#if 1
        else if(pos[1] < '0' || pos[1] > '9') // "score" is 0~10
        {
            return 3;
        }
#endif
        pos = strstr(result, "\"id\":");
        if(pos)
        {
            pos += 5;
            printf("wakeup by %s\n", res[pos[0]-'0']);
            return (pos[0]-'0');
        }

    }
    return 4;
}

int cb_ivw_msg_proc( const char *sessionID, int msg, int param1, int param2, const void *info, void *userData )
{
    int ret;
    if (MSP_IVW_MSG_ERROR == msg) //唤醒出错消息
    {

        printf("\n\nMSP_IVW_MSG_ERROR errCode = %d\n\n", param1);
    }
    else if (MSP_IVW_MSG_WAKEUP == msg) //唤醒成功消息
    {

        printf("\n\nMSP_IVW_MSG_WAKEUP result = %s\n\n", info);
        if(g_status == 0)
        {
            ret = is_real_wakeup((char*)info);
            if(ret>=0 && ret<=2)
            {
                printf("唤醒成功\n");
                if(ret == 0)
                {
                    write(accept_fd,"wake_up0",strlen("wake_up0"));
                    g_status = 1;
                }
                else if(ret == 1)
                {
                    write(accept_fd,"wake_up1",strlen("wake_up1"));
                }
                else if(ret == 2)
                {
                    write(accept_fd,"wake_up2",strlen("wake_up2"));
                    g_status = 1;
                }
            }
            else if(ret == 3)
            {
                printf("唤醒疑问\n");
                write(accept_fd,"abnormal_wakeup",strlen("abnormal_wakeup"));
                //g_status = 3;
            }
            else
                printf("fack wake up!\n");
        }
    }
    return 0;
}

int init_pcm_source_server()
{
    int ret;       
    int len;    

    static char recv_buf[1024];  
    socklen_t clt_addr_len;   
    struct sockaddr_un clt_addr;    
    struct sockaddr_un srv_addr;    

    socket_fd=socket(PF_UNIX,SOCK_STREAM,0);    
    if(socket_fd<0)    
    {    
        printf("cannot create communication socket");    
        return 1;    
    }      

    // 设置服务器参数    
    srv_addr.sun_family=AF_UNIX;    
    strncpy(srv_addr.sun_path,CAN_SERVICE,sizeof(srv_addr.sun_path)-1);    
    unlink(CAN_SERVICE);    

    // 绑定socket地址   
    ret=bind(socket_fd,(struct sockaddr*)&srv_addr,sizeof(srv_addr));    
    if(ret==-1)    
    {    
        printf("cannot bind server socket");    
        close(socket_fd);    
        unlink(CAN_SERVICE);    
        return 1;    
    }    

    // 监听     
    ret=listen(socket_fd,1);    
    if(ret==-1)    
    {    
        printf("cannot listen the client connect request");    
        close(socket_fd);    
        unlink(CAN_SERVICE);    
        return 1;    
    }    

    // 接受connect请求   
    len=sizeof(clt_addr);    
    accept_fd=accept(socket_fd,(struct sockaddr*)&clt_addr,&len);    
    if(accept_fd<0)    
    {    
        printf("cannot accept client connect request");    
        close(socket_fd);    
        unlink(CAN_SERVICE);    
        return 1;    
    }	
    return 0;    
}

int get_pcm_frame_from_socket(char *recv_buf)
{
    int size = read(accept_fd,recv_buf,LIXIAN_FRAME_LEN*10);
    //printf("read buffer size= %d\n", size);
    if(size <= 0)
        return 1;
    else
        return 0;
}

#define FRAME_LEN 256

struct local_voice_param 
{
    int m_last_bytes;
    int m_started;
    int m_last_status;
    int m_last_continue;
    VadVars *vadstate;
    int rec_buffer[FRAME_LEN*2];//16bit
    int m_vad_count;

};
static struct local_voice_param local_param;

int parse_one_frame()
{
    int i;
    int temp;
    float indata[FRAME_LEN];
    int vad;
    local_param.m_vad_count++;
    if(local_param.m_vad_count > 62*8)  // 62 frames per second, 8s.
    {
        printf("vad_max meet??????????????????????????????\n");
        return 1;
    }
    for(i =0; i< FRAME_LEN; i++) {
        temp = 0;
        memcpy(&temp, local_param.rec_buffer+ 2*i, 2); // mono
        indata[i]=(short)temp;
        if (indata[i] > 65535/2)
            indata[i] = indata[i]-65536;
    }
    //ALOGE("parse_one_frame~");

    vad = wb_vad(local_param.vadstate, indata);
    //printf("%d", vad);
    if(vad != local_param.m_last_status) {
        local_param.m_last_status = vad;
        local_param.m_last_continue = 0;
    } else {
        local_param.m_last_continue++;
    }
    local_param.m_last_status = vad;

    if(local_param.m_started == 0 && vad && local_param.m_last_continue > 2)
        local_param.m_started = 1;
    if(local_param.m_started && vad == 0 && local_param.m_last_continue > 60)
    {
        return 1; // stop now;
    }
    return 0;
}

int vad_process(unsigned char *buffer, int bytes)
{
    int cur_start = 0;
    //printf("请说语音指令...\n");
    if(bytes + local_param.m_last_bytes < FRAME_LEN*2) {
        memcpy(local_param.rec_buffer + local_param.m_last_bytes, buffer, bytes);
        local_param.m_last_bytes  += bytes;
    } else {
        do {
            memcpy(local_param.rec_buffer + local_param.m_last_bytes, buffer + cur_start, FRAME_LEN*2 - local_param.m_last_bytes);
            cur_start = cur_start + (FRAME_LEN*2 - local_param.m_last_bytes);
            bytes = bytes - (FRAME_LEN*2 - local_param.m_last_bytes);
            if(parse_one_frame())
            {
                return 1;
            }

            local_param.m_last_bytes = 0;
        }while(bytes + local_param.m_last_bytes >= FRAME_LEN*2);
        if(bytes) {
            memcpy(local_param.rec_buffer + local_param.m_last_bytes, buffer + cur_start, bytes);
            local_param.m_last_bytes = bytes;
        }
    }
    return 0;
}

static NsHandle *pNS_inst = NULL;  
int do_pcm_nsx_init()
{
    if (0 != WebRtcNsx_Create(&pNS_inst))  
    {  
        printf("Noise_Suppression WebRtcNs_Create err! \n");  
        return 1;
    }  

    if (0 != WebRtcNsx_Init(pNS_inst, 16000))  
    {  
        printf("Noise_Suppression WebRtcNs_Init err! \n");  
        return 1;
    }  

    if (0 != WebRtcNsx_set_policy(pNS_inst, 1))  
    {  
        printf("Noise_Suppression WebRtcNs_set_policy err! \n");  
        return 1;
    }
    return 0;
}

void do_pcm_nsx_exit()
{
    WebRtcNsx_Free(pNS_inst);
}

void do_pcm_nsx_process(char *pInBuffer)
{  
    int nRet = 0;  

    int i;
    short shBufferIn[160] = { 0 };  
    short shBufferOut[160] = { 0 };
    int nFileSize = LIXIAN_FRAME_LEN*10;
    do  
    {  
        for (i = 0; i < nFileSize; i += 320)  
        {  
            if (nFileSize - i >= 320)  
            {
                memcpy(shBufferIn, (char*)(pInBuffer + i), 160 * sizeof(short));  

                if (0 == WebRtcNsx_Process(pNS_inst, shBufferIn, NULL, shBufferOut, NULL))  
                {  
                    memcpy(pInBuffer + i, shBufferOut, 160 * sizeof(short));  
                } 
                else
                    printf("WebRtcNsx_Process error !!\n");
            }  
        } 
    } while (0);
}  

void save_file(FILE *fp, char* buf, int size )
{
    fwrite(buf, 1, size, fp);
}
void start_ivw(const char *grammar_list, const char* session_begin_params)
{
    const char *session_id = NULL;
    int err_code = MSP_SUCCESS;
    FILE *f_aud = NULL;
    int audio_stat = MSP_AUDIO_SAMPLE_CONTINUE;
    //char *audio_buffer=NULL;
    char sse_hints[128];
    char *pcm_buffer = (char*)malloc(6400); // every frame is 6400
    int count=0;
    session_id=QIVWSessionBegin(grammar_list, session_begin_params, &err_code);
    if (err_code != MSP_SUCCESS)
    {
        printf("QIVWSessionBegin failed! error code:%d\n",err_code);
        goto exit;
    }

    err_code = QIVWRegisterNotify(session_id, cb_ivw_msg_proc,NULL);
    printf(".");
    if (err_code != MSP_SUCCESS)
    {
        snprintf(sse_hints, sizeof(sse_hints), "QIVWRegisterNotify errorCode=%d", err_code);
        printf("QIVWRegisterNotify failed! error code:%d\n",err_code);
        goto exit;
    }

    printf("ivw init done.\n");
    if(init_pcm_source_server()) // block here until
    {
        printf("socket init error!\n");
    }
    printf("socket client connect done.\n");

    audio_stat = MSP_AUDIO_SAMPLE_FIRST;
    long len = 10*LIXIAN_FRAME_LEN; //16k音频，10帧 （时长200ms）

    // test for vad
    local_param.m_last_bytes = 0;
    local_param.m_started = 0;
    local_param.m_vad_count = 0;
    local_param.m_last_status = 0;
    local_param.m_last_continue = 0;
    memset(local_param.rec_buffer, 0 , sizeof(local_param.rec_buffer));
    wb_vad_init(&(local_param.vadstate));

    //FILE *fp; 
    //fp = fopen("record.pcm",  "wb"); 
    while(1)
    {
        if(get_pcm_frame_from_socket(pcm_buffer))
        {
            //fclose(fp);
            break;
        }

        //do ns:
        do_pcm_nsx_process(pcm_buffer);
        //save_file(fp, pcm_buffer,6400);
        if(g_status == 0)
        {
            //printf("csid=%s,count=%d,aus=%d\n",session_id, count++, audio_stat);
            //printf("请说唤醒词...\n");

            err_code = QIVWAudioWrite(session_id, (const void *)&pcm_buffer[0], len, audio_stat);
            if (MSP_SUCCESS != err_code)
            {
                printf("QIVWAudioWrite failed! error code:%d\n",err_code);
                snprintf(sse_hints, sizeof(sse_hints), "QIVWAudioWrite errorCode=%d", err_code);
                goto exit;
            }
#if 0
            if (MSP_AUDIO_SAMPLE_LAST == audio_stat)
            {
                break;
            }
#endif
            audio_stat = MSP_AUDIO_SAMPLE_CONTINUE;
        }
        else if(g_status == 1 && vad_process(pcm_buffer, LIXIAN_FRAME_LEN*10))//return 1 if vad stop, else return 0
        {
            printf("\nVAD detect!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            local_param.m_last_bytes = 0;
            local_param.m_started = 0;
            local_param.m_vad_count = 0;
            local_param.m_last_status = 0;
            local_param.m_last_continue = 0;
            g_status = 0;
            write(accept_fd,"vad_det",strlen("vad_det"));
        }
#if 0

#endif
        //sleep_ms(200); //模拟人说话时间间隙，10帧的音频时长为200ms  //从socket取得，不需要这个
    }
    snprintf(sse_hints, sizeof(sse_hints), "success");

    return ;
exit:
    if (NULL != session_id)
    {
        QIVWSessionEnd(session_id, sse_hints);
    }
    if (NULL != f_aud)
    {
        fclose(f_aud);
    }

}

int main(int argc, char* argv[])
{
    int         ret       = MSP_SUCCESS;
    const char *lgi_param = "appid = 5a09a164,engine_start = ivw,ivw_res_path =fo|res/ivw/wakeupresource.jet, work_dir = ."; //使用唤醒需要在此设置engine_start = ivw,ivw_res_path =fo|xxx/xx 启动唤醒引擎
    const char *ssb_param = "ivw_threshold=0:-30;0:-30;0:-30,sst=wakeup";

    if(do_pcm_nsx_init())
        return 1;

    ret = MSPLogin(NULL, NULL, lgi_param);
    if (MSP_SUCCESS != ret)
    {
        printf("MSPLogin failed, error code: %d.\n", ret);
        goto exit ;//登录失败，退出登录
    }
    printf("\n###############################################################################################################\n");
    printf("## 请注意，唤醒语音需要根据唤醒词内容自行录制并重命名为宏IVW_AUDIO_FILE_NAME所指定名称，存放在bin/audio文件里##\n");
    printf("###############################################################################################################\n\n");
    //run_ivw(NULL, IVW_AUDIO_FILE_NAME, ssb_param); 
    start_ivw(NULL , ssb_param);
    //sleep_ms(2000);
exit:
    //printf("按任意键退出 ...\n");
    //getchar();
    MSPLogout(); //退出登录
    do_pcm_nsx_exit();
    return 0;
}
