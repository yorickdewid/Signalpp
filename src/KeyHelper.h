#ifndef _KEYHELPER_H_
#define _KEYHELPER_H_

#include "Config.h"
#include "CryptoProvider.h"

namespace signalpp {

namespace KeyHelper {

static const short generateRegistrationId() {
    unsigned char *randId = (unsigned char *)CryptoProvider::getRandomBytes(1);
    unsigned short int registrationId = (short)(((unsigned char)*randId) << 8);
    return registrationId & 0x3fff;
}

} // namespace KeyHelper

} // namespace signalpp

#endif // _KEYHELPER_H_

