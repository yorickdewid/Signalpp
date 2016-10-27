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

	static const short generateRegistrationId();

	static std::string serializeKeyPair(ec_key_pair *key_pair);
	static ec_key_pair *deserializeKeyPair(std::string& str);

	static void print_hex_public_key(ec_public_key *key);
	static void print_hex_private_key(ec_private_key *key);

	inline static void print_hex_key_pair(ec_key_pair *key_pair) {
		print_hex_public_key(ec_key_pair_get_public(key_pair));
		print_hex_private_key(ec_key_pair_get_private(key_pair));
	}

	static std::string encodePublicKey(ec_public_key *key, bool escape = false);

	ec_key_pair *generatePreKey();
	ec_key_pair *generateSignedPreKey(signal_buffer **signature, ec_key_pair *identityKeyPair);
};

} // namespace signalpp

#endif // _KEYHELPER_H_
