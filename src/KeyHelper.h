#ifndef _KEYHELPER_H_
#define _KEYHELPER_H_

#include "Config.h"
#include "CryptoProvider.h"

namespace signalpp {

class KeyHelper {
	signal_context *context;

public:
	KeyHelper() {
		signal_context_create(&context, 0);//TODO: move

		CryptoProvider::hook(context);
	}

	~KeyHelper() {
		signal_context_destroy(context);//TODO: move
	}

	static const short generateRegistrationId() {
		unsigned char *randId = (unsigned char *)CryptoProvider::getRandomBytes(1);
		unsigned short int registrationId = (short)(((unsigned char)*randId) << 8);
		return registrationId & 0x3fff;
	}

	ec_key_pair *generatePreKey() {
		ec_key_pair *key_pair;

		int result = curve_generate_key_pair(context, &key_pair);
		if (result) {
			SIGNAL_LOG_ERROR << "curve_generate_key_pair() failed";
			return nullptr; //TODO: throw
		}

		return key_pair;
	}

};

} // namespace signalpp

#endif // _KEYHELPER_H_

