#include "Logger.h"
#include "Helper.h"
#include "Base64.h"
#include "KeyHelper.h"

using namespace signalpp;

const short KeyHelper::generateRegistrationId() {
	unsigned char *randId = (unsigned char *)CryptoProvider::getRandomBytes(1);
	unsigned short int registrationId = (short)(((unsigned char)*randId) << 8);
	return registrationId & 0x3fff;
}

std::string KeyHelper::serializeKeyPair(ec_key_pair *key_pair) {
	signal_buffer *pub_buffer;
	signal_buffer *priv_buffer;
	ec_public_key_serialize(&pub_buffer, ec_key_pair_get_public(key_pair));
	ec_private_key_serialize(&priv_buffer, ec_key_pair_get_private(key_pair));

	std::string out;
	out += std::string((const char *)signal_buffer_data(pub_buffer), signal_buffer_len(pub_buffer));
	out += "$&&$";
	out += std::string((const char *)signal_buffer_data(priv_buffer), signal_buffer_len(priv_buffer));

	signal_buffer_free(pub_buffer);
	signal_buffer_free(priv_buffer);

	return out;
}

ec_key_pair *KeyHelper::deserializeKeyPair(std::string& str) {
	ec_key_pair *key_pair;
	ec_public_key *public_key = nullptr;
	ec_private_key *private_key = nullptr;

	size_t div = str.find("$&&$");

	std::string pubkey = str.substr(0, div);
	std::string privkey = str.substr(div + 4);

	int result = curve_decode_point(&public_key, (const uint8_t *)pubkey.data(), pubkey.size(), nullptr);
	if (result) {
		SIGNAL_LOG_ERROR << "Cannot decode public key";
		return nullptr; //TODO: throw
	}

	result = curve_decode_private_point(&private_key, (const uint8_t *)privkey.data(), privkey.size(), nullptr);
	if (result) {
		SIGNAL_LOG_ERROR << "Cannot decode private key";
		return nullptr; //TODO: throw
	}

	ec_key_pair_create(&key_pair, public_key, private_key);

	return key_pair;
}

//TODO: debug only
void KeyHelper::print_hex_public_key(ec_public_key *key) {
	signal_buffer *buffer;
	ec_public_key_serialize(&buffer, key);

	uint8_t *data = signal_buffer_data(buffer);
	int len = signal_buffer_len(buffer);
	
	SIGNAL_LOG_DEBUG << "Public key: ";

	int i;
	for (i = 0; i < len; ++i) {
		printf("%02x", data[i]);
	}

	printf("\n");

	signal_buffer_free(buffer);
}

//TODO: debug only
void KeyHelper::print_hex_private_key(ec_private_key *key) {
	signal_buffer *buffer;
	ec_private_key_serialize(&buffer, key);

	uint8_t *data = signal_buffer_data(buffer);
	int len = signal_buffer_len(buffer);
	
	SIGNAL_LOG_DEBUG << "Private key: ";

	int i;
	for (i = 0; i < len; ++i) {
		printf("%02x", data[i]);
	}

	printf("\n");

	signal_buffer_free(buffer);
}

std::string KeyHelper::encodePublicKey(ec_public_key *key, bool escape) {
	signal_buffer *buffer;
	ec_public_key_serialize(&buffer, key);

	uint8_t *data = signal_buffer_data(buffer);
	int len = signal_buffer_len(buffer);

	std::string out;
	if (escape)
		out = Url::Encode(Base64::EncodeRaw((unsigned char const *)data, (unsigned int)len));
	else
		out = Base64::EncodeRaw((unsigned char const *)data, (unsigned int)len);

	signal_buffer_free(buffer);

	return out;
}

ec_key_pair *KeyHelper::generatePreKey() {
	ec_key_pair *key_pair;

	int result = curve_generate_key_pair(context, &key_pair);
	if (result) {
		SIGNAL_LOG_ERROR << "curve_generate_key_pair() failed";
		return nullptr; //TODO: throw
	}

	return key_pair;
}

ec_key_pair *KeyHelper::generateSignedPreKey(signal_buffer **signature, ec_key_pair *identityKeyPair) {
	ec_key_pair *key_pair;
	signal_buffer *message;

	int result = curve_generate_key_pair(context, &key_pair);
	if (result) {
		SIGNAL_LOG_ERROR << "curve_generate_key_pair() failed";
		return nullptr; //TODO: throw
	}

	/* Use public key as message */
	ec_public_key_serialize(&message, ec_key_pair_get_public(key_pair));

	result = curve_calculate_signature(context, signature,
		ec_key_pair_get_private(identityKeyPair),
		signal_buffer_data(message), signal_buffer_len(message));
	if (result) {
		SIGNAL_LOG_ERROR << "curve_calculate_signature() failed";
		return nullptr; //TODO: throw
	}

	signal_buffer_free(message);

	return key_pair;
}
