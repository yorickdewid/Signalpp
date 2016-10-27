#ifndef _ACCOUNT_MANAGER_H_
#define _ACCOUNT_MANAGER_H_

#include "Api.h"
#include "Storage.h"
#include "PreKeyResult.h"

#include <signal_protocol.h>

#include <functional>
#include <memory>

namespace signalpp {

class AccountManager {
	std::unique_ptr<TextSecureServer> m_server;
	StorageContainer *m_storage = nullptr;

public:
    AccountManager(class StorageContainer *storage,
    					const std::string& url,
						const unsigned short ports[],
						const std::string& username,
						const std::string& password)
	: m_server(new TextSecureServer(url, username, password, ports))
	, m_storage(storage) {
    }

	inline void requestVoiceVerification(const std::string& number) {
	    m_server->requestVerificationVoice(number);
	}

	inline void requestSMSVerification(const std::string& number) {
	    m_server->requestVerificationSMS(number);
	}

	bool registerSingleDevice() {
		return false;
	}

	bool registerSecondDevice(std::function<void (const std::string&)> setProvisioningUrl,
								std::function<std::string(const std::string&)> confirmNumber);

	void refreshPreKeys() {}
	void createAccount(const std::string& number,
									const std::string& provisioningCode,
									ec_key_pair *identityKeyPair,
									const std::string& deviceName,
									const std::string& userAgent);
	prekey::result generateKeys(size_t count = 100);
};

} // namespace signalpp

#endif // _ACCOUNT_MANAGER_H_
