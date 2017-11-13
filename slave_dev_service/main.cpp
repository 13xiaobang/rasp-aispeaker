#define LOG_TAG "ai_speaker_server"
#include <utils/Log.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <cutils/properties.h>
#include <libaispeakercore/ai_speaker_types.h>
#include <libaispeakercore/main_process.h>
#include <libaispeakermgr/AiSpeakerManagerService.h>
//#include <test_tmserver.h>
using namespace android;

int main(int argc, char** argv)
{
    (char**) argv;
    if (argc == 1) {
        ALOGI("ai speaker Manager Server init.\n");
        sp<ProcessState> proc(ProcessState::self());
        sp<IServiceManager> sm = defaultServiceManager();
        ALOGI("ai speaker Manager: %p", sm.get());

        if (sm->checkService(String16("aispeaker.manager")) != NULL) {
            ALOGW("ai speaker Manager has already started!, return.\n");
            return 0;
        }
        ai_speaker_main_process();
        AiSpeakerManagerService::instantiate();
        ProcessState::self()->startThreadPool();
        IPCThreadState::self()->joinThreadPool();
    } else {
        ALOGI("AI speaker Manager Test Client.\n");
        //ai_speaker_server_cmd(argc - 1, argv + 1);
    }
    ALOGI("AI speaker Manager close, should never come here.\n");
    return 0;
}
