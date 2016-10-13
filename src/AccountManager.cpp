#include "AccountManager.h"

using namespace signal;

void AccountManager::registerSecondDevice(std::function<void(const std::string&)> setProvisioningUrl, std::function<void()> confirmNumber) {
	/*var socket =*/ m_server->getProvisioningSocket();
	
	// call crypto.getPublicKey()

	setProvisioningUrl("tsdevice:/?uuid=<UUID>&pub_key=<PUBKEY>");

	// call WebSocketResource(socket, lambda)

	generateKeys();
	m_server->registerKeys();
	registrationDone();
}
