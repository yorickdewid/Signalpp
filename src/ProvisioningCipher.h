#ifndef _PROVISIONINGCIPHER_H_
#define _PROVISIONINGCIPHER_H_

#include <signal_protocol.h>

namespace signal {

class ProvisioningCipher {
	signal_context *context;
	ec_key_pair *key_pair;

  public:
  	ProvisioningCipher();
  	~ProvisioningCipher();

	void decrypt() {}
	char *getPublicKey();
};

} // namespace signal

#endif // _PROVISIONINGCIPHER_H_
