#ifndef _SIGNAL_PROTOCOL_STORE_H_
#define _SIGNAL_PROTOCOL_STORE_H_

#include "Storage.h"

#include <unordered_map>

namespace signalpp {

class SignalProtocolStore {
	std::unordered_multimap<std::string, std::string> store;

	void put(std::string key, std::string value) {
		if (key.empty() || value.empty())
			return;//throw new Error("Tried to store undefined/null");
		store.insert(std::pair<std::string, std::string>(key, value));
	}

	std::string get(std::string key, std::string defaultValue = "") {
		if (key.empty())
			return "";//throw new Error("Tried to get value for undefined/null key");

		if (store.find(key) == store.end()) {
			return defaultValue;
		}

		return store.find(key)->second;
	}

	void remove(std::string key) {
		if (key.empty())
			return;//throw new Error("Tried to remove value for undefined/null key");
		store.erase(key);
	}

public:
	std::string getIdentityKeyPair() {
		return get("identityKey");
	}	

	std::string getLocalRegistrationId() {
		return get("registrationId");
	}

	bool isTrustedIdentity(std::string identifier, std::string identityKey) {
		if (identifier.empty()) {
			return false;//throw new error("tried to check identity key for undefined/null key");
		}
		
		auto trusted = get("identityKey" + identifier);
		if (trusted.empty()) {
			// return Promise.resolve(true);
			return true;
		}
		// return Promise.resolve(identityKey === trusted);
		return !trusted.compare(identityKey);
	}

	std::string loadIdentityKey(std::string identifier) {
		if (identifier.empty())
			return "";//throw new Error("Tried to get identity key for undefined/null key");

		return get("identityKey" + identifier);
	}

	void saveIdentity(std::string identifier, std::string identityKey) {
		if (identifier.empty())
			return;//throw new Error("Tried to put identity key for undefined/null key");
		
		put("identityKey" + identifier, identityKey);
	}

	/* Returns a prekeypair object or undefined */
	std::string loadPreKey(int keyId) {
		return get("25519KeypreKey" + std::to_string(keyId));
	}

	void storePreKey(int keyId, std::string keyPair) {
		put("25519KeypreKey" + std::to_string(keyId), keyPair);
	}

	void removePreKey(int keyId) {
		remove("25519KeypreKey" + std::to_string(keyId));
	}

	/* Returns a signed keypair object or undefined */
	std::string loadSignedPreKey(int keyId) {
		return get("25519KeysignedKey" + std::to_string(keyId));
	}
	
	void storeSignedPreKey(int keyId, std::string keyPair) {
		put("25519KeysignedKey" + std::to_string(keyId), keyPair);
	}
	
	void removeSignedPreKey(int keyId) {
		remove("25519KeysignedKey" + std::to_string(keyId));
	}

	std::string loadSession(std::string identifier) {
		return get("session" + identifier);
	}
	
	void storeSession(std::string identifier, std::string record) {
		put("session" + identifier, record);
	}

	void removeAllSessions(std::string identifier) {
		remove("session" + identifier);
		// for (auto& key : store) {
			// if (key.first.compare(0, 7, "session")) {
				// store.erase(key.first);
			// }
			// if (key.match(RegExp("session" + identifier.replace('\+','\\\+') + '.+'))) {
			// 	delete this.store[key];
			// }
		// }
	}

	std::vector<int> getDeviceIds(std::string& identifier) {
		std::vector<int> deviceIds;

		for (auto& key : store) {
			if (key.first.compare(0, 7, "session")) {
				// store.erase(key.first);
				deviceIds.push_back(17);
			}

			// if (key.match(RegExp("session" + identifier.replace('\+','\\\+') + '.+'))) {
				// deviceIds.push_back(parseInt(key.split('.')[1]));
			// }
		}

		return deviceIds;
	}
};


} // namespace signalpp

#endif // _SIGNAL_PROTOCOL_STORE_H_
