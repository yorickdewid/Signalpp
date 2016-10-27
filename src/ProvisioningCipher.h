#ifndef _PROVISIONINGCIPHER_H_
#define _PROVISIONINGCIPHER_H_

#include <signal_protocol.h>

#include "DeviceMessages.pb.h"

namespace signalpp {

struct ProvisionInfo {
	ec_key_pair *identityKeyPair;
	std::string number;
	std::string provisioningCode;
	std::string userAgent;
};

class ProvisioningCipher {
	signal_context *context;
	ec_key_pair *key_pair;

  public:
  	ProvisioningCipher();
  	~ProvisioningCipher();

	ProvisionInfo decrypt(textsecure::ProvisionEnvelope& provisionEnvelope);
	ec_public_key *getPublicKey();
};

} // namespace signalpp

#endif // _PROVISIONINGCIPHER_H_
