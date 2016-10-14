#include "AccountManager.h"
#include "ProvisioningCipher.h"

#include "WebsocketResource.h"

#include <sstream>

using namespace signal;

bool AccountManager::registerSecondDevice(std::function<void(const std::string&)> setProvisioningUrl,
											std::function<std::string(const std::string&)> confirmNumber) {
	ProvisioningCipher provisioningCipher;

	/* Open socket */	
	int socket = m_server->getProvisioningSocket();

	m_server->getProvisioningSocket();

	WebSocketResource(socket, [&] (IncomingWebSocketRequest request) {

        if (request.path == "/v1/address" && request.verb == "PUT") {
        	SIGNAL_LOG_DEBUG << "UUID provisioning message";
            //var proto = textsecure.protobuf.ProvisioningUuid.decode(request.body);

			std::ostringstream os;
			os << "tsdevice:/?uuid=";
			os << "<UUID>";
			os << "&pub_key=";
			os << provisioningCipher.getPublicKey();
			
			/* Call provision hook */
			setProvisioningUrl(os.str());

            request.respond(200, "OK");
        } else if (request.path == "/v1/message" && request.verb == "PUT") {
        	SIGNAL_LOG_DEBUG << "UUID provisioning message";
            //var envelope = textsecure.protobuf.ProvisionEnvelope.decode(request.body, 'binary');
            request.respond(200, "OK");
            // socket.close();
            // var provisionMessage = provisioningCipher.decrypt(envelope);
            std::string deviceName = confirmNumber("+31641074371");
            SIGNAL_LOG_DEBUG << "Client device name: " << deviceName;
            // var deviceName = confirmNumber(provisionMessage.number);
			// return createAccount(
			// 	provisionMessage.number,
			// 	provisionMessage.provisioningCode,
			// 	provisionMessage.identityKeyPair,
			// 	deviceName
			// );
        } else {
        	SIGNAL_LOG_WARNING << "Unknown websocket message";
        }

	});

	generateKeys();
	m_server->registerKeys();
	
	return true;
}

void AccountManager::createAccount(const std::string& number, int provisioningCode, const std::string& deviceName) {

}
