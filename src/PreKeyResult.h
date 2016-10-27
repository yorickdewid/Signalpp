#ifndef _PREKEY_RESULT_H_
#define _PREKEY_RESULT_H_

#include <signal_protocol.h>

#include <vector>

namespace signalpp {

namespace prekey {

struct preKeyPair {
	unsigned int keyId;
	ec_public_key *publicKey;
};

struct signedPreKey {
	unsigned int keyId;
	ec_public_key *publicKey;
	signal_buffer *signature;
};

struct result {
	ec_public_key *identityKey;
	std::vector<preKeyPair> preKeys;
	struct signedPreKey signedPreKey;
};

} // namespace prekey

} // namespace signalpp

#endif // _PREKEY_RESULT_H_
