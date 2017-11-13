/***************************************************************************
 *
 * Copyright (c) 2016 Whaley Ltd. All Rights Reserved
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF Whaley Ltd.
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code.
 *
 ****************************************************************************/
#ifndef __AISPEAKER_MANAGER_SERVICE_H__
#define __AISPEAKER_MANAGER_SERVICE_H__

#include <stdint.h>
#include <sys/types.h>
//#include <string>
#include <utils/Errors.h>
#include <utils/Log.h>
//#include <string>
#include <libaispeakermgr/IAiSpeakerManagerService.h>

namespace android{
class AiSpeakerManagerService: public BnAiSpeakerManagerService
{
public:
    static  void        instantiate();
/*
    // IAiSpeakerManagerService interface
    virtual status_t    setMode(uint32_t mode);
    virtual status_t    getMode(uint32_t *mode);
    virtual status_t    pollEvent(uint32_t *event);
    virtual status_t    getStr(std::string& str);
    virtual status_t    getPcm(char **pcm, int32_t *size);
*/
    // official interface:

    virtual status_t enquiry(void);
    virtual status_t open(void);
    virtual status_t close(void);
    virtual status_t read(char **pcm, int32_t *size);

private:
                        AiSpeakerManagerService();
    virtual             ~AiSpeakerManagerService();
};

};

#endif // __AISPEAKER_MANAGER_SERVICE_H__
