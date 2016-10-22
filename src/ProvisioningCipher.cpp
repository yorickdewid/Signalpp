#include "Logger.h"
#include "Helper.h"
#include "CryptoProvider.h"
#include "ProvisioningCipher.h"
#include "Base64.h"

using namespace signalpp;

void print_hex_public_key(ec_public_key *key) {
	signal_buffer *buffer;
	ec_public_key_serialize(&buffer, key);

	uint8_t *data = signal_buffer_data(buffer);
	int len = signal_buffer_len(buffer);
	
	SIGNAL_LOG_DEBUG << "New keypair with pubkkey: ";

	int i;
	for (i = 0; i < len; ++i) {
		printf("%02x", data[i]);
	}

	signal_buffer_free(buffer);
}

std::string encodePublicKey(ec_public_key *key) {
	signal_buffer *buffer;
	ec_public_key_serialize(&buffer, key);

	uint8_t *data = signal_buffer_data(buffer);
	int len = signal_buffer_len(buffer);

	std::string out = Url::Encode(Base64::EncodeRaw((unsigned char const *)data, (unsigned int)len));

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

std::string ProvisioningCipher::getPublicKey() {
	int result = curve_generate_key_pair(context, &key_pair);
	if (result) {
		SIGNAL_LOG_ERROR << "curve_generate_key_pair() failed";
		return nullptr; //TODO: throw
	}

	ec_public_key *public_key = ec_key_pair_get_public(key_pair);

	return encodePublicKey(public_key);
}
