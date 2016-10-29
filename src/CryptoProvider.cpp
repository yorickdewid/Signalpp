#include "Logger.h"
#include "CryptoProvider.h"

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#include <memory.h>

using namespace signalpp;

#define AES_KEY_LENGTH 32

/*
 * Random byte generator
 */
static int randomGenerator(uint8_t *data, size_t len, void *user_data) {
	if (RAND_bytes(data, len)) {
		return 0;
	}

	return SG_ERR_UNKNOWN;
}

/*
 * SHA-256 digest
 */
static int HMAC_SHA256Init(void **hmac_context, const uint8_t *key, size_t key_len, void *user_data) {
	HMAC_CTX *ctx = (HMAC_CTX *)malloc(sizeof(HMAC_CTX));
	if (!ctx) {
		return SG_ERR_NOMEM;
	}

	HMAC_CTX_init(ctx);
	*hmac_context = ctx;

	if (HMAC_Init_ex(ctx, key, key_len, EVP_sha256(), 0) != 1) {
		return SG_ERR_UNKNOWN;
	}

	return 0;
}

static int HMAC_SHA256Update(void *hmac_context, const uint8_t *data, size_t data_len, void *user_data) {
	HMAC_CTX *ctx = (HMAC_CTX *)hmac_context;
	int result = HMAC_Update(ctx, data, data_len);
	return (result == 1) ? 0 : -1;
}

static int HMAC_SHA256Final(void *hmac_context, signal_buffer **output, void *user_data) {
	int result = 0;
	unsigned char md[EVP_MAX_MD_SIZE];
	unsigned int len = 0;
	HMAC_CTX *ctx = (HMAC_CTX *)hmac_context;

	if(HMAC_Final(ctx, md, &len) != 1) {
		return SG_ERR_UNKNOWN;
	}

	signal_buffer *output_buffer = signal_buffer_create(md, len);
	if(!output_buffer) {
		result = SG_ERR_NOMEM;
		goto complete;
	}

	*output = output_buffer;

complete:
	return result;
}

static void HMAC_SHA256Cleanup(void *hmac_context, void *user_data) {
	if (hmac_context) {
		HMAC_CTX *ctx = (HMAC_CTX *)hmac_context;
		HMAC_CTX_cleanup(ctx);
		free(ctx);
	}
}

/*
 * SHA-512 digest
 */

static int SHA512Digest(signal_buffer **output, const uint8_t *data, size_t data_len, void *user_data) {
	int result = 0;
	signal_buffer *buffer = 0;
	SHA512_CTX ctx;

	buffer = signal_buffer_alloc(SHA512_DIGEST_LENGTH);
	if(!buffer) {
		result = SG_ERR_NOMEM;
		goto complete;
	}

	result = SHA512_Init(&ctx);
	if(!result) {
		result = SG_ERR_UNKNOWN;
		goto complete;
	}

	result = SHA512_Update(&ctx, data, data_len);
	if(!result) {
		result = SG_ERR_UNKNOWN;
		goto complete;
	}

complete:
	if(buffer) {
		result = SHA512_Final(signal_buffer_data(buffer), &ctx);
		if(!result) {
			result = SG_ERR_UNKNOWN;
		}
	}

	if (result < 0) {
		signal_buffer_free(buffer);
	}
	else {
		*output = buffer;
	}
	return result;
}

/*
 * Symmetric encryptio/decryption routines
 */
static const EVP_CIPHER *AESCipher(int cipher, size_t key_len) {
	if (cipher == SG_CIPHER_AES_CBC_PKCS5) {
		if (key_len == 16) {
			return EVP_aes_128_cbc();
		} else if (key_len == 24) {
			return EVP_aes_192_cbc();
		} else if (key_len == 32) {
			return EVP_aes_256_cbc();
		}
	} else if (cipher == SG_CIPHER_AES_CTR_NOPADDING) {
		if (key_len == 16) {
			return EVP_aes_128_ctr();
		} else if (key_len == 24) {
			return EVP_aes_192_ctr();
		} else if (key_len == 32) {
			return EVP_aes_256_ctr();
		}
	}

	return 0;
}

static int encrypt_raw(signal_buffer **output,
		int cipher,
		const uint8_t *key, size_t key_len,
		const uint8_t *iv, size_t iv_len,
		const uint8_t *plaintext, size_t plaintext_len,
		void *user_data) {
	int result = 0;
	uint8_t *out_buf = nullptr;

	const EVP_CIPHER *evp_cipher = AESCipher(cipher, key_len);
	if (!evp_cipher) {
		fprintf(stderr, "invalid AES mode or key size: %zu\n", key_len);
		return SG_ERR_UNKNOWN;
	}

	if (iv_len != 16) {
		fprintf(stderr, "invalid AES IV size: %zu\n", iv_len);
		return SG_ERR_UNKNOWN;
	}

	if ((int)plaintext_len > INT_MAX - EVP_CIPHER_block_size(evp_cipher)) {
		fprintf(stderr, "invalid plaintext length: %zu\n", plaintext_len);
		return SG_ERR_UNKNOWN;
	}

	EVP_CIPHER_CTX ctx;
	EVP_CIPHER_CTX_init(&ctx);

	{
		result = EVP_EncryptInit_ex(&ctx, evp_cipher, 0, key, iv);
		if(!result) {
			fprintf(stderr, "cannot initialize cipher\n");
			result = SG_ERR_UNKNOWN;
			goto encrypt_complete;
		}

		if(cipher == SG_CIPHER_AES_CTR_NOPADDING) {
			result = EVP_CIPHER_CTX_set_padding(&ctx, 0);
			if(!result) {
				fprintf(stderr, "cannot set padding\n");
				result = SG_ERR_UNKNOWN;
				goto encrypt_complete;
			}
		}

		out_buf = (uint8_t *)malloc(sizeof(uint8_t) * (plaintext_len + EVP_CIPHER_block_size(evp_cipher)));
		if(!out_buf) {
			fprintf(stderr, "cannot allocate output buffer\n");
			result = SG_ERR_NOMEM;
			goto encrypt_complete;
		}

		int out_len = 0;
		result = EVP_EncryptUpdate(&ctx, out_buf, &out_len, plaintext, plaintext_len);
		if (!result) {
			fprintf(stderr, "cannot encrypt plaintext\n");
			result = SG_ERR_UNKNOWN;
			goto encrypt_complete;
		}

		int final_len = 0;
		result = EVP_EncryptFinal_ex(&ctx, out_buf + out_len, &final_len);
		if (!result) {
			fprintf(stderr, "cannot finish encrypting plaintext\n");
			result = SG_ERR_UNKNOWN;
			goto encrypt_complete;
		}

		*output = signal_buffer_create(out_buf, out_len + final_len);
	}

encrypt_complete:
	EVP_CIPHER_CTX_cleanup(&ctx);
	if (out_buf) {
		free(out_buf);
	}

	return result;
}

static int decrypt_raw(signal_buffer **output,
		int cipher,
		const uint8_t *key, size_t key_len,
		const uint8_t *iv, size_t iv_len,
		const uint8_t *ciphertext, size_t ciphertext_len,
		void *user_data) {
	int result = 0;
	uint8_t *out_buf = 0;

	const EVP_CIPHER *evp_cipher = AESCipher(cipher, key_len);
	if(!evp_cipher) {
		fprintf(stderr, "invalid AES mode or key size: %zu\n", key_len);
		return SG_ERR_INVAL;
	}

	if (iv_len != 16) {
		fprintf(stderr, "invalid AES IV size: %zu\n", iv_len);
		return SG_ERR_INVAL;
	}

	if ((int)ciphertext_len > INT_MAX - EVP_CIPHER_block_size(evp_cipher)) {
		fprintf(stderr, "invalid ciphertext length: %zu\n", ciphertext_len);
		return SG_ERR_UNKNOWN;
	}

	EVP_CIPHER_CTX ctx;
	EVP_CIPHER_CTX_init(&ctx);

	{
		result = EVP_DecryptInit_ex(&ctx, evp_cipher, 0, key, iv);
		if(!result) {
			fprintf(stderr, "cannot initialize cipher\n");
			result = SG_ERR_UNKNOWN;
			goto decrypt_complete;
		}

		if (cipher == SG_CIPHER_AES_CTR_NOPADDING) {
			result = EVP_CIPHER_CTX_set_padding(&ctx, 0);
			if(!result) {
				fprintf(stderr, "cannot set padding\n");
				result = SG_ERR_UNKNOWN;
				goto decrypt_complete;
			}
		}

		out_buf = (uint8_t *)malloc(sizeof(uint8_t) * (ciphertext_len + EVP_CIPHER_block_size(evp_cipher)));
		if (!out_buf) {
			fprintf(stderr, "cannot allocate output buffer\n");
			result = SG_ERR_UNKNOWN;
			goto decrypt_complete;
		}

		int out_len = 0;
		result = EVP_DecryptUpdate(&ctx, out_buf, &out_len, ciphertext, ciphertext_len);
		if (!result) {
			fprintf(stderr, "cannot decrypt ciphertext\n");
			result = SG_ERR_UNKNOWN;
			goto decrypt_complete;
		}

		int final_len = 0;
		result = EVP_DecryptFinal_ex(&ctx, out_buf + out_len, &final_len);
		if(!result) {
			ERR_print_errors_fp(stderr);

			fprintf(stderr, "cannot finish decrypting ciphertext\n");
			result = SG_ERR_UNKNOWN;
			goto decrypt_complete;
		}

		*output = signal_buffer_create(out_buf, out_len + final_len);
	}

decrypt_complete:
	EVP_CIPHER_CTX_cleanup(&ctx);
	if (out_buf) {
		free(out_buf);
	}
	return result;
}

/* 
 * Autopadding operations
 */

unsigned char *aes_cbc_encrypt(unsigned char *key, unsigned char *iv, unsigned char *plaintext, size_t plaintext_len) {
    AES_KEY encryption_key;

    unsigned char *output = (unsigned char *)calloc(plaintext_len, sizeof(unsigned char));

    AES_set_encrypt_key(key, AES_KEY_LENGTH * 8, &encryption_key);
    AES_cbc_encrypt(plaintext, output, plaintext_len, &encryption_key, iv, AES_ENCRYPT);

    return output;
}

unsigned char *aes_cbc_decrypt(unsigned char *key, unsigned char *iv, unsigned char *ciphertext, size_t ciphertext_len) {
    AES_KEY decryption_key;
    
    unsigned char *output = (unsigned char *)calloc(ciphertext_len, sizeof(unsigned char));

    AES_set_decrypt_key(key, AES_KEY_LENGTH * 8, &decryption_key);
    AES_cbc_encrypt(ciphertext, output, ciphertext_len, &decryption_key, iv, AES_DECRYPT);

    return output;
}

void CryptoProvider::hook(signal_context *context) {
	signal_crypto_provider provider = {
		.random_func = randomGenerator,
		.hmac_sha256_init_func = HMAC_SHA256Init,
		.hmac_sha256_update_func = HMAC_SHA256Update,
		.hmac_sha256_final_func = HMAC_SHA256Final,
		.hmac_sha256_cleanup_func = HMAC_SHA256Cleanup,
		.sha512_digest_func = SHA512Digest,
		.encrypt_func = encrypt_raw,
		.decrypt_func = decrypt_raw,
		.user_data = nullptr
	};

	signal_context_set_crypto_provider(context, &provider);
}

std::string CryptoProvider::HMAC(std::string& key, std::string& data) {
	int result = 0;
	signal_buffer *output_buffer = nullptr;
	size_t output_len = 0;
	std::string hash;

	void *hmac_context = nullptr;
	void *user_data = nullptr;
	result = HMAC_SHA256Init(&hmac_context, (const uint8_t *)key.data(), key.size(), user_data);
	if (result < 0) {
		goto hmac_complete;
	}

	result = HMAC_SHA256Update(hmac_context, (const uint8_t *)data.data(), data.size(), user_data);
	if (result < 0) {
		goto hmac_complete;
	}

	result = HMAC_SHA256Final(hmac_context, &output_buffer, user_data);
	if (result < 0) {
		goto hmac_complete;
	}

	hash = std::string((char *)signal_buffer_data(output_buffer), signal_buffer_len(output_buffer));

hmac_complete:
	HMAC_SHA256Cleanup(hmac_context, user_data);
	signal_buffer_free(output_buffer);

	if (result < 0) {
		SIGNAL_LOG_ERROR << "Cannot calculate HMAC";
		return nullptr;//TODO: throw
	}

	return hash;
}

bool CryptoProvider::verifyMAC(std::string& data, std::string& key, std::string& mac, size_t length) {
	std::string calculatedMAC = HMAC(key, data);

	if (mac.size() != length || calculatedMAC.size() != length) {
		SIGNAL_LOG_ERROR << "Bad MAC length";
		return false;//TODO: throw
	}

	if (!mac.compare(calculatedMAC)) {
		SIGNAL_LOG_INFO << "MAC CHECK";
		return true;
	} else {
		SIGNAL_LOG_INFO << "MAC WHOOPS";
	}

	return false;
}

std::string CryptoProvider::decrypt(const std::string& key, const std::string& ciphertext, const std::string& iv) {
	unsigned char _key[AES_KEY_LENGTH];
	unsigned char _iv[AES_BLOCK_SIZE];
	unsigned char _ciphertext[128];

	memcpy(_key, key.data(), AES_KEY_LENGTH);
	memcpy(_iv, iv.data(), AES_BLOCK_SIZE);
	memset(_ciphertext, '\0', 128);
	memcpy(_ciphertext, ciphertext.data(), ciphertext.size());

	unsigned char *dec = aes_cbc_decrypt(_key, _iv, _ciphertext, ciphertext.size());

	std::string plaintext = std::string((const char *)dec, ciphertext.size());

	free(dec);

	return plaintext;
}

char *CryptoProvider::getRandomBytes(size_t size) {
	uint8_t *data = (uint8_t *)malloc(size);
	randomGenerator(data, size, nullptr);
	return (char *)data;
}
