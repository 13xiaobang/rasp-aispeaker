#define LOG_TAG "ai_speaker_server"
#include <utils/Log.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <libaispeakercore/virtual_key.h>
#include <stdlib.h>
#include <errno.h>

int run_system(const char * cmdstring)
{
    pid_t pid;
    int status;

    if ((pid = fork())<0) {
        status = -1;
    } else if (pid == 0) {
        //execv(path, cmdstring);
        execl("/system/bin/busybox", "sh", "-c", cmdstring, (char *)0);
        exit(127);
    } else {
        while (waitpid(pid, &status, 0) < 0){
            if (errno != EINTR) {
                break;
            }
       }
    }
    return status;
}


void send_vk_event(enum VK_EVENT event)
{
    //uinput_write_key(uifd, KEY_CODE_AI_SPEAKER, event);
    if(event)
    {
        run_system( "input keyevent --down 5067");
    }
    else
    {
        run_system( "input keyevent --up 5067");
    }

}
