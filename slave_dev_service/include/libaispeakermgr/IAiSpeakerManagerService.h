/***************************************************************************
 *
 * Copyright (c) 2016 Whaley Ltd. All Rights Reserved
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF Whaley Ltd.
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code.
 *
 ****************************************************************************/
#ifndef __IAISPEAKER_MANAGER_SERVICE_H__
#define __IAISPEAKER_MANAGER_SERVICE_H__

#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
//#include <string>
namespace android {
class IAiSpeakerManagerService: public IInterface
{
public:
    DECLARE_META_INTERFACE(AiSpeakerManagerService);
/*
    virtual status_t    setMode(uint32_t mode) = 0;
    virtual status_t    getMode(uint32_t *mode) = 0;
    virtual status_t    pollEvent(uint32_t *event) = 0;
    virtual status_t    getStr(std::string &str) = 0;
    virtual status_t    getPcm(char **pcm, int32_t *size) = 0;
*/
    //official interface:
    virtual status_t    enquiry() = 0;
    virtual status_t    open() = 0;
    virtual status_t    close() = 0;
    virtual status_t    read(char **pcm, int32_t *size) = 0;
};

// ----------------------------------------------------------------------------

class BnAiSpeakerManagerService: public BnInterface<IAiSpeakerManagerService>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

}; // namespace android

#endif // __IAISPEAKER_MANAGER_SERVICE_H__
