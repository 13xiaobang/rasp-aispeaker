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
#include <sys/types.h>

#include <binder/Parcel.h>
#include <utils/String8.h>

#include <utils/Errors.h>
#include <utils/Log.h>
#include <libaispeakercore/ai_speaker_types.h>
#include <libaispeakermgr/IAiSpeakerManagerService.h>
namespace android {

enum {
	/*
    SET_MODE = IBinder::FIRST_CALL_TRANSACTION,
    GET_MODE,
    POLL_EVENT,
    GET_STR,
    GET_PCM,
    */
    ENQUIRY = IBinder::FIRST_CALL_TRANSACTION,
    OPEN,
    CLOSE,
    READ
};

class BpAiSpeakerManagerService: public BpInterface<IAiSpeakerManagerService>
{
public:
    BpAiSpeakerManagerService(const sp<IBinder>& impl)
        : BpInterface<IAiSpeakerManagerService>(impl)
    {
    }
/*
    virtual status_t    setMode(uint32_t mode)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAiSpeakerManagerService::getInterfaceDescriptor());
        data.writeInt32((int32_t)mode);
        remote()->transact(SET_MODE, data, &reply);
        return reply.readInt32();
    }

    virtual status_t    getMode(uint32_t *mode)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAiSpeakerManagerService::getInterfaceDescriptor());
        remote()->transact(GET_MODE, data, &reply);
        status_t status =  reply.readInt32();
        *mode = (uint32_t)reply.readInt32();
        return status;
    }

	virtual status_t    pollEvent(uint32_t *event)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAiSpeakerManagerService::getInterfaceDescriptor());
        remote()->transact(POLL_EVENT, data, &reply);
        status_t status =  reply.readInt32();
        *event = (uint32_t)reply.readInt32();
        return status;
    }

	virtual status_t    getStr(std::string& str)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAiSpeakerManagerService::getInterfaceDescriptor());
        remote()->transact(GET_STR, data, &reply);
        status_t status =  reply.readInt32();
		String8 str8(reply.readString8());
		str = str8.string();
        return status;
    }

	virtual status_t    getPcm(char **pcm, int32_t *size)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAiSpeakerManagerService::getInterfaceDescriptor());
        remote()->transact(GET_PCM, data, &reply);
        status_t status =  reply.readInt32();
		*size =  reply.readInt32();
		Parcel::ReadableBlob blob;
		reply.readBlob(*size, &blob);
		memcpy(*pcm, blob.data(), *size);
		blob.release();
        return status;
    }
*/

	virtual status_t	enquiry(void)
	{
		Parcel data, reply;
		data.writeInterfaceToken(IAiSpeakerManagerService::getInterfaceDescriptor());
		remote()->transact(ENQUIRY, data, &reply);
		status_t status =  reply.readInt32();
		return status;
	}

	virtual status_t	open(void)
	{
		Parcel data, reply;
		data.writeInterfaceToken(IAiSpeakerManagerService::getInterfaceDescriptor());
		remote()->transact(OPEN, data, &reply);
		status_t status =  reply.readInt32();
		return status;
	}

	virtual status_t	close(void)
	{
		Parcel data, reply;
		data.writeInterfaceToken(IAiSpeakerManagerService::getInterfaceDescriptor());
		remote()->transact(CLOSE, data, &reply);
		status_t status =  reply.readInt32();
		return status;
	}

	virtual status_t	read(char **pcm, int32_t *size)
	{
		Parcel data, reply;
        data.writeInterfaceToken(IAiSpeakerManagerService::getInterfaceDescriptor());

		data.writeInt32(*size);
        remote()->transact(READ, data, &reply);
        status_t status =  reply.readInt32();
		*size =  reply.readInt32();
		Parcel::ReadableBlob blob;
		reply.readBlob(*size, &blob);
		memcpy(*pcm, blob.data(), *size);
		blob.release();
        return status;
	}

};

IMPLEMENT_META_INTERFACE(AiSpeakerManagerService, "android.hardware.IAiSpeakerManagerService");

// ----------------------------------------------------------------------

status_t BnAiSpeakerManagerService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code) {
/*
        case SET_MODE: {
            ALOGI("IAiSpeakerManagerService :  SET_MODE");
            CHECK_INTERFACE(BpAiSpeakerManagerService, data, reply);
            uint32_t mode = 0;
            mode = (uint32_t)data.readInt32();
            reply->writeInt32(setMode(mode));
            return NO_ERROR;
        } break;
         case GET_MODE: {
            ALOGI("IAiSpeakerManagerService :  GET_MODE");
            CHECK_INTERFACE(BpAiSpeakerManagerService, data, reply);
            uint32_t mode = 0;
            reply->writeInt32(getMode(&mode));
            reply->writeInt32(mode);
            return NO_ERROR;
        } break;
		 case POLL_EVENT: {
            ALOGI("IAiSpeakerManagerService :  POLL_EVENT");
            CHECK_INTERFACE(BpAiSpeakerManagerService, data, reply);
            uint32_t event = 0;
            reply->writeInt32(pollEvent(&event));
            reply->writeInt32(event);
            return NO_ERROR;
        } break;
		 case GET_STR: {
            ALOGI("IAiSpeakerManagerService :  GET_STR");
            CHECK_INTERFACE(BpAiSpeakerManagerService, data, reply);
            std::string str;
            reply->writeInt32(getStr(str));
            reply->writeString8(String8(str.c_str()));
            return NO_ERROR;
        } break;
		 case GET_PCM: {
            ALOGI("IAiSpeakerManagerService :  GET_PCM");
            CHECK_INTERFACE(BpAiSpeakerManagerService, data, reply);
			int32_t size;
			char *pcm;

            reply->writeInt32(getPcm(&pcm, &size));
			reply->writeInt32(size);
			Parcel::WritableBlob blob;
			reply->writeBlob(size, &blob);
			memcpy(blob.data(), pcm, size);
            blob.release();

            return NO_ERROR;
        } break;
*/
		 case ENQUIRY: {
            ALOGI("IAiSpeakerManagerService :  ENQUIRY");
            CHECK_INTERFACE(BpAiSpeakerManagerService, data, reply);
            reply->writeInt32(enquiry());
            return NO_ERROR;
        } break;
		 case OPEN: {
            ALOGI("IAiSpeakerManagerService :  OPEN");
            CHECK_INTERFACE(BpAiSpeakerManagerService, data, reply);
            reply->writeInt32(open());
            return NO_ERROR;
        } break;
		 case CLOSE: {
            ALOGI("IAiSpeakerManagerService :  CLOSE");
            CHECK_INTERFACE(BpAiSpeakerManagerService, data, reply);
            reply->writeInt32(close());
            return NO_ERROR;
        } break;
		 case READ: {
            ALOGI("IAiSpeakerManagerService :  READ");
            CHECK_INTERFACE(BpAiSpeakerManagerService, data, reply);
			int32_t size;
			char *pcm;

			int require_size = data.readInt32();
            reply->writeInt32(read(&pcm, &require_size));
			reply->writeInt32(require_size);
			Parcel::WritableBlob blob;
#ifdef CFLAGS_MSTAR_838_938
			reply->writeBlob(require_size, false, &blob);
#else
			reply->writeBlob(require_size, &blob);
#endif
			memcpy(blob.data(), pcm, require_size);
            blob.release();

            return NO_ERROR;
        } break;
        default:
            ALOGV("UNKNOWN_CODE");
            return BBinder::onTransact(code, data, reply, flags);
    }
    return NO_ERROR;
}

}; // namespace android
