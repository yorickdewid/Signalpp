#ifndef _SESSION_BUILDER_H_
#define _SESSION_BUILDER_H_

#include "CryptoProvider.h"
#include "ProtocolAddress.h"
#include "ProtocolStore.h"

#include <signal_protocol.h>
#include <session_builder.h>
#include <session_state.h>
#include <session_cipher.h>
#include <protocol.h>

namespace signalpp {

class SessionBuilder {

	/* needs to be stored..*/

	signal_context *context = nullptr;
	signal_protocol_store_context *store_context = nullptr;
	session_builder *m_session_builder = nullptr;
	session_cipher *m_session_cipher = nullptr;
	ProtocolAddress *m_address = nullptr;

public:
	SessionBuilder(ProtocolAddress *address) : m_address(address) {
		signal_context_create(&context, 0);//TODO: move

		CryptoProvider::hook(context);
		ProtocolStore::hook(&store_context, context);

		int result = session_builder_create(&m_session_builder, store_context, &m_address->address, context);
		if (result) {
			//SIGNAL_LOG_ERROR << "session_builder_create() failed";
			std::cerr << "session_builder_create() failed" << std::endl;
		}
	}

	~SessionBuilder() {
		delete m_address;

		signal_context_destroy(context);
	}

	/* supply from storage */
	void processPreKey(nlohmann::json device) {
		session_pre_key_bundle *pre_key = nullptr;

		/* Convert prekey */
		std::string preKeyPublicKey = device["preKey"]["publicKey"].get<std::string>();
		ec_public_key *prekey_public_key = nullptr;

		int result = curve_decode_point(&prekey_public_key, (const uint8_t *)preKeyPublicKey.data(), preKeyPublicKey.size(), context);
		if (result) {
			//SIGNAL_LOG_ERROR << "Cannot decode public key";
			std::cerr << "Cannot decode public key" << std::endl;
			return; //TODO: throw
		}

		/* Convert signed prekey */
		std::string signedPreKeyPublicKey = device["signedPreKey"]["publicKey"].get<std::string>();
		ec_public_key *signed_prekey_public_key = nullptr;

		result = curve_decode_point(&signed_prekey_public_key, (const uint8_t *)signedPreKeyPublicKey.data(), signedPreKeyPublicKey.size(), context);
		if (result) {
			//SIGNAL_LOG_ERROR << "Cannot decode public key";
			std::cerr << "Cannot decode public key" << std::endl;
			return; //TODO: throw
		}

		/* Convert signature */
		std::string signature = device["signedPreKey"]["signature"].get<std::string>();

		/* Convert identity key */
		std::string identityKey = device["identityKey"].get<std::string>();
		ec_public_key *identity_public_key = nullptr;

		/* this is THEIR identity key */
		result = curve_decode_point(&identity_public_key, (const uint8_t *)identityKey.data(), identityKey.size(), context);
		if (result) {
			//SIGNAL_LOG_ERROR << "Cannot decode public key";
			std::cerr << "Cannot decode public key" << std::endl;
			return; //TODO: throw
		}

		result = session_pre_key_bundle_create(&pre_key,
			device["registrationId"], /* Registration id */
			device["deviceId"], /* Device ID */
			device["preKey"]["keyId"], /* Prekey id */
			prekey_public_key,
			device["signedPreKey"]["keyId"], /* Signed pre key id */
			signed_prekey_public_key,
			(const uint8_t *)signature.data(),
			signature.size(),
			identity_public_key);
		if (result) {
			//SIGNAL_LOG_ERROR << "session_pre_key_bundle_create() failed";
			std::cerr << "session_pre_key_bundle_create() failed" << std::endl;
			return;
		}

		/* Process device pre key bundle */
		result = session_builder_process_pre_key_bundle(m_session_builder, pre_key);
		if (result) {
			//SIGNAL_LOG_ERROR << "session_builder_process_pre_key_bundle() failed";
			std::cerr << "session_builder_process_pre_key_bundle() failed" << std::endl;
			return;
		}

		// TESTING //

		/* Check that we can load the session state and verify its version */
		// result = signal_protocol_session_contains_session(store_context, &m_address.address);
		// if (result != 1) {
		// 	SIGNAL_LOG_ERROR << "signal_protocol_session_contains_session() failed";
		// 	return;
		// }

		// session_record *loaded_record = nullptr;
		// session_state *loaded_record_state = nullptr;
		// result = signal_protocol_session_load_session(store_context, &loaded_record, &m_address.address);
		
		// loaded_record_state = session_record_get_state(loaded_record);

		// if (session_state_get_session_version(loaded_record_state) != 3) {
		// 	SIGNAL_LOG_ERROR << "Invalid session version";
		// 	return;
		// }

		SIGNAL_LOG_INFO << "All good!";
		// SIGNAL_UNREF(loaded_record);
	}

	std::pair<std::string, int> encryptToDevice(std::string plaintext) {
		int result = session_cipher_create(&m_session_cipher, store_context, &m_address->address, context);
		if (result) {
			//SIGNAL_LOG_ERROR << "session_cipher_create() failed";
			std::cerr << "session_cipher_create() failed" << std::endl;
			return std::make_pair("", 0);//TODO: throw
		}

		ciphertext_message *ciphertext = nullptr;
		result = session_cipher_encrypt(m_session_cipher, (uint8_t *)plaintext.data(), plaintext.size(), &ciphertext);
		if (result) {
			//SIGNAL_LOG_ERROR << "session_cipher_encrypt() failed";
			std::cerr << "session_cipher_encrypt() failed" << std::endl;
			return std::make_pair("", 0);//TODO: throw
		}

		signal_buffer *buffer = ciphertext_message_get_serialized(ciphertext);
		int type = ciphertext_message_get_type(ciphertext);

		return std::make_pair(std::string((char *)signal_buffer_data(buffer), signal_buffer_len(buffer)), type);
	}

	unsigned int getRegistrationId() {
		session_record *loaded_record = nullptr;
		int result = signal_protocol_session_load_session(store_context, &loaded_record, &m_address->address);
		
		session_state *loaded_record_state = session_record_get_state(loaded_record);
		unsigned int id = session_state_get_local_registration_id(loaded_record_state);

		SIGNAL_UNREF(loaded_record);

		return id;
	}

	unsigned int getRegistrationRemoteId() {
		unsigned int id = 0;

		session_cipher_get_remote_registration_id(m_session_cipher, &id);

		return id;
	}

	// void processV3(record, message) {
	// 	var preKeyPair, signedPreKeyPair, session;
	// 	return this.storage.isTrustedIdentity(
	// 		this.remoteAddress.getName(), message.identityKey.toArrayBuffer()
	// 	).then(function(trusted) {
	// 		if (!trusted) {
	// 			var e = new Error('Unknown identity key');
	// 			e.identityKey = message.identityKey.toArrayBuffer();
	// 			throw e;
	// 		}
	// 		return Promise.all([
	// 			this.storage.loadPreKey(message.preKeyId),
	// 			this.storage.loadSignedPreKey(message.signedPreKeyId),
	// 		]).then(function(results) {
	// 			preKeyPair       = results[0];
	// 			signedPreKeyPair = results[1];
	// 		});
	// 	}.bind(this)).then(function() {
	// 		session = record.getSessionByBaseKey(message.baseKey);
	// 		if (session) {
	// 		  console.log("Duplicate PreKeyMessage for session");
	// 		  return;
	// 		}

	// 		session = record.getOpenSession();

	// 		if (signedPreKeyPair === undefined) {
	// 			// Session may or may not be the right one, but if its not, we
	// 			// can't do anything about it ...fall through and let
	// 			// decryptWhisperMessage handle that case
	// 			if (session !== undefined && session.currentRatchet !== undefined) {
	// 				return;
	// 			} else {
	// 				throw new Error("Missing Signed PreKey for PreKeyWhisperMessage");
	// 			}
	// 		}

	// 		if (session !== undefined) {
	// 			record.archiveCurrentState();
	// 		}
	// 		if (message.preKeyId && !preKeyPair) {
	// 			console.log('Invalid prekey id', message.preKeyId);
	// 		}
	// 		return this.initSession(false, preKeyPair, signedPreKeyPair,
	// 			message.identityKey.toArrayBuffer(),
	// 			message.baseKey.toArrayBuffer(), undefined, message.registrationId
	// 		).then(function(new_session) {
	// 			// Note that the session is not actually saved until the very
	// 			// end of decryptWhisperMessage ... to ensure that the sender
	// 			// actually holds the private keys for all reported pubkeys
	// 			record.updateSessionState(new_session, message.registrationId);
	// 			return this.storage.saveIdentity(this.remoteAddress.getName(), message.identityKey.toArrayBuffer()).then(function() {
	// 			  return message.preKeyId;
	// 			});
	// 		}.bind(this));
	// 	}.bind(this));
	// }
  
  // TODO: private
 //  void initSession(isInitiator,
	// 				ourEphemeralKey,
	// 				ourSignedKey,
	// 				theirIdentityPubKey,
	// 				theirEphemeralPubKey,
	// 				theirSignedPubKey,
	// 				registrationId) {
	// return this.storage.getIdentityKeyPair().then(function(ourIdentityKey) {
	// 	if (isInitiator) {
	// 		if (ourSignedKey !== undefined) {
	// 			throw new Error("Invalid call to initSession");
	// 		}
	// 		ourSignedKey = ourEphemeralKey;
	// 	} else {
	// 		if (theirSignedPubKey !== undefined) {
	// 			throw new Error("Invalid call to initSession");
	// 		}
	// 		theirSignedPubKey = theirEphemeralPubKey;
	// 	}

	// 	var sharedSecret;
	// 	if (ourEphemeralKey === undefined || theirEphemeralPubKey === undefined) {
	// 		sharedSecret = new Uint8Array(32 * 4);
	// 	} else {
	// 		sharedSecret = new Uint8Array(32 * 5);
	// 	}

	// 	for (var i = 0; i < 32; i++) {
	// 		sharedSecret[i] = 0xff;
	// 	}

	// 	return Promise.all([
	// 		Internal.crypto.ECDHE(theirSignedPubKey, ourIdentityKey.privKey),
	// 		Internal.crypto.ECDHE(theirIdentityPubKey, ourSignedKey.privKey),
	// 		Internal.crypto.ECDHE(theirSignedPubKey, ourSignedKey.privKey)
	// 	]).then(function(ecRes) {
	// 		if (isInitiator) {
	// 			sharedSecret.set(new Uint8Array(ecRes[0]), 32);
	// 			sharedSecret.set(new Uint8Array(ecRes[1]), 32 * 2);
	// 		} else {
	// 			sharedSecret.set(new Uint8Array(ecRes[0]), 32 * 2);
	// 			sharedSecret.set(new Uint8Array(ecRes[1]), 32);
	// 		}
	// 		sharedSecret.set(new Uint8Array(ecRes[2]), 32 * 3);

	// 		if (ourEphemeralKey !== undefined && theirEphemeralPubKey !== undefined) {
	// 			return Internal.crypto.ECDHE(
	// 				theirEphemeralPubKey, ourEphemeralKey.privKey
	// 			).then(function(ecRes4) {
	// 				sharedSecret.set(new Uint8Array(ecRes4), 32 * 4);
	// 			});
	// 		}
	// 	}).then(function() {
	// 		return Internal.HKDF(sharedSecret.buffer, new ArrayBuffer(32), "WhisperText");
	// 	}).then(function(masterKey) {
	// 		var session = {
	// 			registrationId: registrationId,
	// 			currentRatchet: {
	// 				rootKey                : masterKey[0],
	// 				lastRemoteEphemeralKey : theirSignedPubKey,
	// 				previousCounter        : 0
	// 			},
	// 			indexInfo: {
	// 				remoteIdentityKey : theirIdentityPubKey,
	// 				closed            : -1
	// 			},
	// 			oldRatchetList: []
	// 		};

	// 		// If we're initiating we go ahead and set our first sending ephemeral key now,
	// 		// otherwise we figure it out when we first maybeStepRatchet with the remote's ephemeral key
	// 		if (isInitiator) {
	// 			session.indexInfo.baseKey = ourEphemeralKey.pubKey;
	// 			session.indexInfo.baseKeyType = Internal.BaseKeyType.OURS;
	// 			return Internal.crypto.createKeyPair().then(function(ourSendingEphemeralKey) {
	// 				session.currentRatchet.ephemeralKeyPair = ourSendingEphemeralKey;
	// 				return this.calculateSendingRatchet(session, theirSignedPubKey).then(function() {
	// 					return session;
	// 				});
	// 			}.bind(this));
	// 		} else {
	// 			session.indexInfo.baseKey = theirEphemeralPubKey;
	// 			session.indexInfo.baseKeyType = Internal.BaseKeyType.THEIRS;
	// 			session.currentRatchet.ephemeralKeyPair = ourSignedKey;
	// 			return session;
	// 		}
	// 	}.bind(this));
	// }.bind(this));
 //  }
  
  //TODO: private
  // void calculateSendingRatchet(session, remoteKey) {
	 //  var ratchet = session.currentRatchet;

	 //  return Internal.crypto.ECDHE(
		//   remoteKey, util.toArrayBuffer(ratchet.ephemeralKeyPair.privKey)
	 //  ).then(function(sharedSecret) {
		//   return Internal.HKDF(
		// 	  sharedSecret, util.toArrayBuffer(ratchet.rootKey), "WhisperRatchet"
		//   );
	 //  }).then(function(masterKey) {
		//   session[util.toString(ratchet.ephemeralKeyPair.pubKey)] = {
		// 	  messageKeys : {},
		// 	  chainKey    : { counter : -1, key : masterKey[1] },
		// 	  chainType   : Internal.ChainType.SENDING
		//   };
		//   ratchet.rootKey = masterKey[0];
	 //  });
  // }

};

} // namespace signalpp

#endif // _SESSION_BUILDER_H_
