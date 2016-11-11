#ifndef _SESSION_CIPHER_H_
#define _SESSION_CIPHER_H_

#include "CryptoProvider.h"
#include "ProtocolAddress.h"
#include "ProtocolStore.h"

#include <signal_protocol.h>

namespace signalpp {

class SessionCipher {
	ProtocolAddress m_address;

public:
	SessionCipher(ProtocolAddress& address) : m_address(address) {
		//
	}

	~SessionCipher() {
		//
	}

	getRecord(encodedNumber) {
		return this.storage.loadSession(encodedNumber).then(function(serialized) {
			if (serialized === undefined) {
				return undefined;
			}
			return Internal.SessionRecord.deserialize(serialized);
		});
	}

	void encrypt(unsigned char *buffer) {
		// buffer = dcodeIO.ByteBuffer.wrap(buffer, encoding).toArrayBuffer();

		var address = m_addess.toString();
		var session, chain;

		var msg = new Internal.protobuf.WhisperMessage();

		var ourIdentityKey = this.storage.getIdentityKeyPair();
		var myRegistrationId = this.storage.getLocalRegistrationId();
		var record = getRecord(address);
		if (!record) {
			throw new Error("No record for " + address);
		}
		
		session = record.getOpenSession();
		if (!session) {
			throw new Error("No session to encrypt message for " + address);
		}

		msg.ephemeralKey = util.toArrayBuffer(session.currentRatchet.ephemeralKeyPair.pubKey);
		chain = session[util.toString(msg.ephemeralKey)];
		if (chain.chainType === Internal.ChainType.RECEIVING) {
			throw new Error("Tried to encrypt on a receiving chain");
		}

		return this.fillMessageKeys(chain, chain.chainKey.counter + 1);
	
		var keys = Internal.HKDF(
			util.toArrayBuffer(chain.messageKeys[chain.chainKey.counter]),
			new ArrayBuffer(32),
			"WhisperMessageKeys");
	
		delete chain.messageKeys[chain.chainKey.counter];
		msg.counter = chain.chainKey.counter;
		msg.previousCounter = session.currentRatchet.previousCounter;

		var ciphertext = Internal.crypto.encrypt(keys[0], buffer, keys[2].slice(0, 16); // key, data, iv
			
		msg.ciphertext = ciphertext;
		var encodedMsg = msg.toArrayBuffer();

		var macInput = new Uint8Array(encodedMsg.byteLength + 33*2 + 1);
		macInput.set(new Uint8Array(util.toArrayBuffer(ourIdentityKey.pubKey)));
		macInput.set(new Uint8Array(util.toArrayBuffer(session.indexInfo.remoteIdentityKey)), 33);
		macInput[33*2] = (3 << 4) | 3;
		macInput.set(new Uint8Array(encodedMsg), 33*2 + 1);

		var message = Internal.crypto.sign(keys[1], macInput.buffer).then(function(mac) {
			var result = new Uint8Array(encodedMsg.byteLength + 9);
			result[0] = (3 << 4) | 3;
			result.set(new Uint8Array(encodedMsg), 1);
			result.set(new Uint8Array(mac, 0, 8), encodedMsg.byteLength + 1);

			record.updateSessionState(session);
			this.storage.storeSession(address, record.serialize());
			return result;
		}
		
		if (session.pendingPreKey !== undefined) {
			var preKeyMsg = new Internal.protobuf.PreKeyWhisperMessage();
			preKeyMsg.identityKey = util.toArrayBuffer(ourIdentityKey.pubKey);
			preKeyMsg.registrationId = myRegistrationId;

			preKeyMsg.baseKey = util.toArrayBuffer(session.pendingPreKey.baseKey);
			preKeyMsg.preKeyId = session.pendingPreKey.preKeyId;
			preKeyMsg.signedPreKeyId = session.pendingPreKey.signedKeyId;

			preKeyMsg.message = message;
			var result = String.fromCharCode((3 << 4) | 3) + util.toString(preKeyMsg.encode());
			return {
				type           : 3,
				body           : result,
				registrationId : session.registrationId
			};

		} else {
			return {
				type           : 1,
				body           : util.toString(message),
				registrationId : session.registrationId
			};
		}

			
	}

};

} // namespace signalpp

#endif // _SESSION_CIPHER_H_
