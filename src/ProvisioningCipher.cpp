#include "Logger.h"
#include "CryptoProvider.h"
#include "ProvisioningCipher.h"

using namespace signal;

char *convert_public_key(ec_public_key *key) {
	signal_buffer *buffer;
	ec_public_key_serialize(&buffer, key);

	uint8_t *data = signal_buffer_data(buffer);
	int len = signal_buffer_len(buffer);
	char *out = (char *)malloc((len * 2) + 1);
	int i;
	for (i = 0; i < len; ++i) {
		sprintf(&out[i*2], "%02x", data[i]);
	}

	out[(len * 2) + 1] = '\0';
	SIGNAL_LOG_DEBUG << "New keypair with pubkkey: " << out;
	signal_buffer_free(buffer);

	return out;
}

ProvisioningCipher::ProvisioningCipher() {
	signal_context_create(&context, 0);

	CryptoProvider::hook(context);
}

ProvisioningCipher::~ProvisioningCipher() {
	// SIGNAL_UNREF(key_pair);

	signal_context_destroy(context);
}

char *ProvisioningCipher::getPublicKey() {
	int result = curve_generate_key_pair(context, &key_pair);
	if (result) {
		SIGNAL_LOG_ERROR << "curve_generate_key_pair() failed";
		return nullptr; //TODO: throw
	}

	ec_public_key *public_key = ec_key_pair_get_public(key_pair);

	return convert_public_key(public_key);
}
