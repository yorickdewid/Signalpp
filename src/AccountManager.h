#ifndef _ACCOUNT_MANAGER_H_
#define _ACCOUNT_MANAGER_H_

#include "Api.h"

#include <functional>
#include <memory>

namespace signal {

class AccountManager {
	std::unique_ptr<TextSecureServer> m_server;

public:
    AccountManager(const std::string& url,
						const unsigned short ports[],
						const std::string& username,
						const std::string& password)
	: m_server(new TextSecureServer(url, username, password, ports)) {
    }

	inline void requestVoiceVerification(const std::string& number) {
	    m_server->requestVerificationVoice(number);
	}

	inline void requestSMSVerification(const std::string& number) {
	    m_server->requestVerificationSMS(number);
	}

	bool registerSingleDevice() { return false; }
	bool registerSecondDevice(std::function<void (const std::string&)> setProvisioningUrl,
								std::function<std::string(const std::string&)> confirmNumber);
	void refreshPreKeys() {}
	void createAccount(const std::string& number, int provisioningCode, const std::string& deviceName);
	void generateKeys() {}
};

} // namespace signal

#endif // _ACCOUNT_MANAGER_H_
