#include "AccountManager.h"
#include "ProvisioningCipher.h"

#include <sstream>

using namespace signal;

bool AccountManager::registerSecondDevice(std::function<void(const std::string&)> setProvisioningUrl, std::function<void()> confirmNumber) {
	ProvisioningCipher crypto;
	/*var socket = m_server->getProvisioningSocket(); */
	
	// call WebSocketResource(socket, lambda)

	std::ostringstream os;
	os << "tsdevice:/?uuid=";
	os << "<UUID>";
	os << "&pub_key=";
	os << crypto.getPublicKey();

	setProvisioningUrl(os.str());

	generateKeys();
	m_server->registerKeys();
	
	return true;
}
