#include "Logger.h"
#include "Helper.h"
#include "Base64.h"
#include "CryptoProvider.h"
#include "ProvisioningCipher.h"
#include "KeyHelper.h"
#include "DeviceMessages.pb.h"

#include <hkdf.h>

using namespace signalpp;

ProvisioningCipher::ProvisioningCipher() {
	signal_context_create(&context, 0);//TODO: move

	CryptoProvider::hook(context);
}

ProvisioningCipher::~ProvisioningCipher() {
	// SIGNAL_UNREF(key_pair);

	signal_context_destroy(context);//TODO: move
}

ec_public_key *ProvisioningCipher::getPublicKey() {
	int result = curve_generate_key_pair(context, &key_pair);
	if (result) {
		SIGNAL_LOG_ERROR << "curve_generate_key_pair() failed";
		return nullptr; //TODO: throw
	}

	return ec_key_pair_get_public(key_pair);
}

ProvisionInfo ProvisioningCipher::decrypt(textsecure::ProvisionEnvelope& provisionEnvelope) {
	ProvisionInfo info;

	std::string masterEphemeral = provisionEnvelope.publickey();
	std::string message = provisionEnvelope.body();

	if (message[0] != 0x1) {
		SIGNAL_LOG_ERROR << "Bad version number on ProvisioningMessage";
		return info; //TODO: throw
	}

	std::string iv = message.substr(1, 16);
	std::string mac = message.substr(message.size() - 32);
	std::string ivAndCiphertext = message.substr(0, message.size() - 32);
	std::string ciphertext = message.substr(16 + 1, message.size() - 32);

	SIGNAL_LOG_DEBUG << "mac: " << Base64::Encode(mac);
	SIGNAL_LOG_DEBUG << "ivAndCiphertext: " << Base64::Encode(ivAndCiphertext);
	SIGNAL_LOG_DEBUG << "ciphertext: " << Base64::Encode(ciphertext);
	SIGNAL_LOG_DEBUG << "iv: " << Base64::Encode(iv);

	/* Derive shared secret */
	uint8_t *shared_secret = nullptr;
	ec_private_key *private_key = ec_key_pair_get_private(key_pair);
	ec_public_key *public_key = nullptr;

	int result = curve_decode_point(&public_key, (const uint8_t *)masterEphemeral.c_str(), masterEphemeral.size(), context); //TODO: c_str() to data()
	if (result) {
		SIGNAL_LOG_ERROR << "Cannot decode public key";
		return info; //TODO: throw
	}

	result = curve_calculate_agreement(&shared_secret, public_key, private_key);
	if (result != 32) {
		SIGNAL_LOG_ERROR << "Agreement failed";
		return info; //TODO: throw
	}

	/* Derive 3 keys */
	hkdf_context *hkdf_context;

	uint8_t salt[32];
	memset(salt, '\0', 32);

	char infoText[] = "TextSecure Provisioning Message";

	result = hkdf_create(&hkdf_context, 3, context);
	if (result) {
		SIGNAL_LOG_ERROR << "Key derivation failed";
		return info; //TODO: throw
	}

	uint8_t *derived_keys = nullptr;
	result = hkdf_derive_secrets(hkdf_context, &derived_keys, //TODO: use cryptoprovider
		shared_secret, 32,
		salt, 32,
		(uint8_t *)infoText, 31,
		96);
	if (result != 96) {
		SIGNAL_LOG_ERROR << "Key derivation failed";
		return info; //TODO: throw
	}

	std::string derivedKeys = std::string((char *)derived_keys, 96);
	std::string key0 = derivedKeys.substr(0, 32);
	std::string key1 = derivedKeys.substr(32, 32);
	std::string key2 = derivedKeys.substr(64, 32);

	SIGNAL_LOG_DEBUG << "key0: " << Base64::Encode(key0);
	SIGNAL_LOG_DEBUG << "key1: " << Base64::Encode(key1);
	SIGNAL_LOG_DEBUG << "key2: " << Base64::Encode(key2);

	/* Verify MAC and decrypt */
	if (CryptoProvider::verifyMAC(ivAndCiphertext, key1, mac, 32)) {
		std::string plaintext = CryptoProvider::decrypt(key0, ciphertext, iv);

		textsecure::ProvisionMessage ProvisionMessage;
		ProvisionMessage.ParseFromString(plaintext);

		SIGNAL_LOG_DEBUG << "identityKeyPrivate size: " << ProvisionMessage.identitykeyprivate().size();
		SIGNAL_LOG_DEBUG << "number: " << ProvisionMessage.number();
		SIGNAL_LOG_DEBUG << "provisioningcode: " << ProvisionMessage.provisioningcode();
		SIGNAL_LOG_DEBUG << "userAgent: " << ProvisionMessage.useragent();

		ec_key_pair *_key_pair;
		ec_public_key *public_key = nullptr;
		ec_private_key *private_key = nullptr;

		result = curve_decode_private_point(&private_key, (const uint8_t *)ProvisionMessage.identitykeyprivate().data(), ProvisionMessage.identitykeyprivate().size(), context);

		result = curve_generate_public_key(&public_key, private_key);

		ec_key_pair_create(&_key_pair, public_key, private_key);

		info.identityKeyPair = _key_pair;
		info.number = ProvisionMessage.number();
		info.provisioningCode = ProvisionMessage.provisioningcode();
		info.userAgent = ProvisionMessage.useragent();

		return info;
	}

	SIGNAL_LOG_ERROR << "Message verification failed";

	// if (derived_keys) {
	// 	free(derived_keys);
	// }
	// SIGNAL_UNREF(hkdf_context);
//

	return info;
}
