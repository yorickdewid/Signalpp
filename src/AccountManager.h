#ifndef _ACCOUNT_MANAGER_H_
#define _ACCOUNT_MANAGER_H_

#include "Api.h"
#include "WebsocketResource.h"

namespace sinl {

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

	void registerSingleDevice() {}
	void registerSecondDevice() {
		/*var socket =*/ m_server->getProvisioningSocket();

		// call crypto.getPublicKey()
		// call WebSocketResource(socket, lambda)

		generateKeys();
		m_server->registerKeys();
		registrationDone();
	}
	void refreshPreKeys() {}
	void createAccount() {}
	void generateKeys() {}
	void registrationDone() {}
};

} // namespace sinl

#endif // _ACCOUNT_MANAGER_H_
