#ifndef _OUTGOING_MESSAGE_H_
#define _OUTGOING_MESSAGE_H_

#include "Api.h"
#include "Storage.h"
#include "Json.h"
#include "Base64.h"
#include "ProtocolAddress.h"
#include "SessionBuilder.h"
// #include "SessionCipher.h"

#include <signal_protocol.h>

namespace signalpp {

class OutgoingMessage {
	std::shared_ptr<TextSecureServer> m_server;
	// SignalProtocolStore protocol;
	std::vector<std::string> successfulNumbers;
	std::map<int, SessionBuilder *> sessions;
	std::string m_message;
	std::vector<int> _deviceIds;
	long int m_timestamp;
	bool m_legacy = false;

	void handleKeyResults(nlohmann::json response, std::string number, std::vector<int> updateDevices) {
		for (auto& device : response["devices"]) {
			device["identityKey"] = response["identityKey"];
				
			_deviceIds.push_back(device["deviceId"].get<int>());

			ProtocolAddress *address = new ProtocolAddress(number, device["deviceId"].get<int>());
			// SessionBuilder *builder = new SessionBuilder(address);
			// sessions.push_back(new SessionBuilder(address));
			// puts("DEALLOC");

			// SIGNAL_LOG_INFO << "ProtocolAddress 1 " << (void *)&address;

			sessions[address->getDeviceId()] = new SessionBuilder(address);
			sessions[address->getDeviceId()]->processPreKey(device);

			// builder->processPreKey(device);
			//TODO: handle errors
			// 	if (error.message === "Identity key changed") {
			// 		error = new textsecure.OutgoingIdentityKeyError(
			// 			number, this.message.toArrayBuffer(),
			// 			this.timestamp, device.identityKey);
			// 		this.registerError(number, "Identity key changed", error);
			// 	}
			// 	throw error;
			// }
		}
	}

public:
	OutgoingMessage(std::shared_ptr<TextSecureServer> server,
					long int timestamp,
					std::vector<std::string>& numbers,
					std::string& message,
					std::function<void()> callback)
		: m_server(server)
		, m_message(message)
		, m_timestamp(timestamp) {
		
		// this.numbers = numbers;
		// this.callback = callback;
		// this.legacy = (message instanceof textsecure.protobuf.DataMessage);

		// this.numbersCompleted = 0;
		// this.errors = [];
		// this.successfulNumbers = [];
	}

	void reloadDevicesAndSend(std::string& number, bool recurse) {
		// MOCK MOCK
		auto deviceIds = _deviceIds;//textsecure.storage.protocol.getDeviceIds(number);// return all device ids
		if (deviceIds.empty()) {
			return;//return this.registerError(number, "Got empty device list when loading device keys", null);
		}

		SIGNAL_LOG_INFO << "deviceIds sz " << deviceIds.size();

		doSendMessage(number, deviceIds, recurse);
	}

	inline size_t getPaddedMessageLength(size_t messageLength) {
		size_t messageLengthWithTerminator = messageLength + 1;
		size_t messagePartCount = messageLengthWithTerminator / 160;

		SIGNAL_LOG_INFO << "messageLengthWithTerminator " << messageLengthWithTerminator;
		SIGNAL_LOG_INFO << "messagePartCount " << messagePartCount;

		/* If not align, add padding block */
		if (messageLengthWithTerminator % 160 != 0) {
			messagePartCount++;
		}

		return messagePartCount * 160;
	}

	/* Only called by doSendMessage */
	std::tuple<std::string, int> transmitMessage(std::string& number, nlohmann::json jsonData, long int timestamp) {
		std::string message = jsonData.dump();

		return m_server->sendMessages(number, message, timestamp);
		//TODO: handle errors
		//     if (e.name === 'HTTPError' && (e.code !== 409 && e.code !== 410)) {
		//         // 409 and 410 should bubble and be handled by doSendMessage
		//         // 404 should throw UnregisteredUserError
		//         // all other network errors can be retried later.
		//         if (e.code === 404) {
		//             throw new textsecure.UnregisteredUserError(number, e);
		//         }
		//         throw new textsecure.SendMessageNetworkError(number, jsonData, e, timestamp);
		//     }
		//     throw e;
	}

	void doSendMessage(std::string& number, std::vector<int> deviceIds, bool recurse) {
		nlohmann::json objects = nlohmann::json::array();

		size_t paddedPlaintextLength = getPaddedMessageLength(m_message.length() + 1);
		auto paddedPlaintext = new char[paddedPlaintextLength - 1];
		memset(paddedPlaintext, '\0', paddedPlaintextLength - 1);
		strncpy(paddedPlaintext, m_message.data(), m_message.size());
		paddedPlaintext[m_message.size()] = 0x80;

		SIGNAL_LOG_INFO << "ciphertext size: " << m_message.size();
		SIGNAL_LOG_INFO << "Padded ciphertext: ";

		unsigned int i = 0;
		for (i=0; i<paddedPlaintextLength - 1; ++i) {
			printf("%d:%02x ", i, (char)paddedPlaintext[i]);
		}

		printf("\n");

		for (int& deviceId : deviceIds) {
		 	auto ciphertext = sessions.at(deviceId)->encryptToDevice(std::string(paddedPlaintext, 159));
			SIGNAL_LOG_INFO << "Ciphertext length " << std::get<0>(ciphertext).length();
			
			unsigned int id = sessions.at(deviceId)->getRegistrationRemoteId();
			auto jsonData = toJSON(deviceId, std::get<0>(ciphertext), id, std::get<1>(ciphertext));
			objects.push_back(jsonData);
		}

		SIGNAL_LOG_INFO << "objects size " << objects.size();

		int code;
		std::string response;
		std::tie(response, code) = transmitMessage(number, objects, m_timestamp);
		if (code == 409) {
			SIGNAL_LOG_INFO << "Missing devices";

			std::vector<int> updateDevices;

			auto res = nlohmann::json::parse(response);
			for (auto& device : res["missingDevices"]) {
				updateDevices.push_back(device.get<int>());
			}
			
			getKeysForNumber(number, updateDevices);
			reloadDevicesAndSend(number, true);
		} else {
			// successfulNumbers.push_back(number);
			// numberCompleted();
		}
	}

	nlohmann::json toJSON(int deviceId, std::string& ciphertext, unsigned int registrationId, int type) {
		nlohmann::json json;
		json["type"] = type;
		json["destinationDeviceId"] = deviceId;
		json["destinationRegistrationId"] = registrationId;

		std::string content = Base64::Encode(ciphertext);
		if (m_legacy) {
			json["body"] = content;
		} else {
			json["content"] = content;
		}

		return json;
	}

	void getKeysForNumber(std::string& number, std::vector<int> updateDevices) {
		if (updateDevices.empty()) {
			auto result = m_server->getKeysForNumber(number);
			if (result == nullptr)
				return;
			handleKeyResults(result, number, updateDevices);
		} else {
			for (int device : updateDevices) {
				auto result = m_server->getKeysForNumber(number, device);
				if (result == nullptr)
					continue;
				handleKeyResults(result, number, updateDevices);
				//TODO: handle errors
				// if (e.name === 'HTTPError' && e.code === 404 && device !== 1) {
				// 	return this.removeDeviceIdsForNumber(number, [device]);
				// } else {
				// 	throw e;
				// }
			}

			return;
		}
	}

	std::vector<int> getStaleDeviceIdsForNumber(std::string& number) {
		std::vector<int> updateDevices;

		// auto deviceIds = protocol.getDeviceIds(number);
		// if (!deviceIds.empty()) {
			// for (auto& deviceId : deviceIds) {
				// auto address = new libsignal.SignalProtocolAddress(number, deviceId);
				// auto sessionCipher = new libsignal.SessionCipher(protocol, address);
				// if (!sessionCipher.hasOpenSession())
				// 	updateDevices.push_back(deviceId);
			// }
		// } else {
			updateDevices.push_back(1);
		// }

		return updateDevices;
	}

	void sendToNumber(std::string& number) {
		SIGNAL_LOG_INFO << "Send to number " << number;

		auto updateDevices = getStaleDeviceIdsForNumber(number);
		getKeysForNumber(number, updateDevices);
		reloadDevicesAndSend(number, true);
		//TODO: handle errors
	}

};

} // namespace signalpp

#endif // _OUTGOING_MESSAGE_H_
