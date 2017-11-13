#ifndef VIRTUAL_KEY_H
#define VIRTUAL_KEY_H
#ifdef __cplusplus
extern "C" {
#endif

enum VK_EVENT {
    VK_PCM_DONE_EVENT=0,
    VK_WAKEUP_EVENT,
};
//int setup_uinput_device();

void send_vk_event(enum VK_EVENT event);

#ifdef __cplusplus
}
#endif

#endif

