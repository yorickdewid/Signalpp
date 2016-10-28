#include "AccountManager.h"
#include "ProvisioningCipher.h"
#include "WebsocketResource.h"
#include "DeviceMessages.pb.h"
#include "CryptoProvider.h"
#include "KeyHelper.h"
#include "Base64.h"

#include <sstream>

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
			os << KeyHelper::encodePublicKey(provisioningCipher.getPublicKey(), true);

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

			/* Feed data inaccount creation routine */
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

	auto result = generateKeys();
	m_server->registerKeys(result);
	
	return true;
}

void AccountManager::createAccount(const std::string& number,
									const std::string& provisioningCode,
									ec_key_pair *identityKeyPair,
									const std::string& deviceName,
									const std::string& userAgent) {
	SIGNAL_LOG_INFO << "Creating new account";

	std::string signalingKey = std::string(CryptoProvider::getRandomBytes(32 + 20), 32 + 20);
	std::string password = Base64::EncodeRaw((const unsigned char *)CryptoProvider::getRandomBytes(16), 16);
	password = password.substr(0, password.size() - 2);
	short int registrationId = KeyHelper::generateRegistrationId();

	SIGNAL_LOG_DEBUG << "Password: " << password;
	SIGNAL_LOG_DEBUG << "registrationId: " << registrationId;

	int deviceId = m_server->confirmCode(number, provisioningCode, password, signalingKey, registrationId, deviceName);
	SIGNAL_LOG_DEBUG << "DeviceID: " << deviceId;

	std::string number_id = number + "." + std::to_string(deviceId);
	SIGNAL_LOG_DEBUG << "number_id: " << number_id;

	m_storage->purge("identityKey");
	m_storage->purge("signaling_key");
	m_storage->purge("password");
	m_storage->purge("registrationId");
	m_storage->purge("number");
	m_storage->purge("number_id");
	m_storage->purge("device_name");
	m_storage->purge("device_id");
	m_storage->purge("userAgent");

	m_storage->put("identityKey", KeyHelper::serializeKeyPair(identityKeyPair));
	m_storage->put("signaling_key", signalingKey);
	m_storage->put("password", password);
	m_storage->put("registrationId", std::to_string(registrationId));
	m_storage->put("number", number);
	m_storage->put("number_id", number_id);
	m_storage->put("device_name", deviceName);
	m_storage->put("device_id", deviceId);
	m_storage->put("userAgent", userAgent);

	m_server->setUsername(number_id);
}

prekey::result AccountManager::generateKeys(size_t count) {
	SIGNAL_LOG_INFO << "Generate prekeys";

	KeyHelper keyHelper;
	prekey::result result;

	unsigned int startId = 0;
	unsigned int signedKeyId = 0;

	m_storage->get("maxPreKeyId", (int&)startId);
	m_storage->get("signedKeyId", (int&)signedKeyId);

	std::string serialKey;
	m_storage->get("identityKey", serialKey);
	ec_key_pair *identityKey = KeyHelper::deserializeKeyPair(serialKey);

	/* Generate prekeys */
	for (unsigned int keyId = startId; keyId < startId + count; ++keyId) {
		ec_key_pair *res = keyHelper.generatePreKey();
		// // store.storePreKey(res.keyId, res.keyPair);//optional?
		prekey::preKeyPair preKeyPair;
		preKeyPair.keyId = keyId;
		preKeyPair.publicKey = ec_key_pair_get_public(res);
		result.preKeys.push_back(preKeyPair);
	}

	/* Generate signed key */
	signal_buffer *signature = nullptr;

	ec_key_pair *res = keyHelper.generateSignedPreKey(&signature, identityKey);

	// signedPreKey signedPreKey;
	result.signedPreKey.keyId = signedKeyId;
	result.signedPreKey.publicKey = ec_key_pair_get_public(res);
	result.signedPreKey.signature = signature;

	/* Attatch public identity key to result */
	result.identityKey = ec_key_pair_get_public(identityKey);

	// store.removeSignedPreKey(signedKeyId - 2);//NEIN
	m_storage->put("maxPreKeyId", startId + count);
	m_storage->put("signedKeyId", signedKeyId + 1);

	return result;
}
