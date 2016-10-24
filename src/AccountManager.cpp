#include "AccountManager.h"
#include "ProvisioningCipher.h"
#include "WebsocketResource.h"
#include "DeviceMessages.pb.h"

#include <sstream>
#include <fstream>
#include "CryptoProvider.h"
using namespace signalpp;

bool AccountManager::registerSecondDevice(std::function<void(const std::string&)> setProvisioningUrl,
											std::function<std::string(const std::string&)> confirmNumber) {
	ProvisioningCipher provisioningCipher;

	GOOGLE_PROTOBUF_VERIFY_VERSION;

	/* Open socket */
	auto socket = m_server->getProvisioningSocket();

	socket->onClose([] {
		SIGNAL_LOG_INFO << "Websocket closed";
	});

	WebSocketResource(socket, [&] (IncomingWebSocketRequest request) {
		if (request.path == "/v1/address" && request.verb == "PUT") {
			SIGNAL_LOG_INFO << "UUID provisioning message";

			textsecure::ProvisioningUuid provUuid;
			provUuid.ParseFromString(request.body);

			std::ostringstream os;
			os << "tsdevice:/?uuid=";
			os << provUuid.uuid();
			os << "&pub_key=";
			os << provisioningCipher.getPublicKey();
			
			/* Call provision hook */
			setProvisioningUrl(os.str());

			request.respond(200, "OK");
		} else if (request.path == "/v1/message" && request.verb == "PUT") {
			SIGNAL_LOG_DEBUG << "ephemeral message";

			textsecure::ProvisionEnvelope envelope;
			envelope.ParseFromString(request.body);
			
			request.respond(200, "OK");
			socket->close();
			
			auto provisionMessage = provisioningCipher.decrypt(envelope);
			std::string deviceName = confirmNumber(provisionMessage.number);
			SIGNAL_LOG_INFO << "Client device name: " << deviceName;

			return createAccount(
				provisionMessage.number,
				provisionMessage.provisioningCode,
				provisionMessage.identityKeyPair,
				deviceName,
				provisionMessage.userAgent
			);
		} else {
			SIGNAL_LOG_WARNING << "Unknown websocket message";
		}

	});

	delete socket;

	generateKeys();
	m_server->registerKeys();
	
	return true;
}

void AccountManager::createAccount(const std::string& number,
									const std::string& provisioningCode,
									ec_key_pair *identityKeyPair,
									const std::string& deviceName,
									const std::string& userAgent) {
	SIGNAL_LOG_INFO << "Creating new account";

}
