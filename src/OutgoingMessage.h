#ifndef _OUTGOING_MESSAGE_H_
#define _OUTGOING_MESSAGE_H_

#include "Api.h"
#include "Storage.h"
#include "Json.h"

#include <signal_protocol.h>

namespace signalpp {

class OutgoingMessage {
	// std::unique_ptr<TextSecureServer> m_server;
	// StorageContainer *m_storage = nullptr;
	std::shared_ptr<TextSecureServer> m_server;

public:
	OutgoingMessage(std::shared_ptr<TextSecureServer> server,
					long int timestamp,
					std::vector<std::string>& numbers,
					std::string& message,
					std::function<void()> callback)
		: m_server(server) {
		
		// this.timestamp = timestamp;
		// this.numbers = numbers;
		// this.message = message; // DataMessage or ContentMessage proto
		// this.callback = callback;
		// this.legacy = (message instanceof textsecure.protobuf.DataMessage);

		// this.numbersCompleted = 0;
		// this.errors = [];
		// this.successfulNumbers = [];
	}

	void handleKeyResults(nlohmann::json response, std::vector<int> updateDevices) {
		// SIGNAL_LOG_INFO << "handleKeyResults response " << response;
		// var handleResult = function(response) {
			// return Promise.all(response.devices.map(function(device) {
				
			// 	device.identityKey = response.identityKey;
				
			// 	if (updateDevices.empty() || updateDevices.indexOf(device.deviceId) > -1) {
			// 		var address = new libsignal.SignalProtocolAddress(number, device.deviceId);
			// 		var builder = new libsignal.SessionBuilder(textsecure.storage.protocol, address);

			// 		result = session_builder_create(&alice_session_builder, alice_store, &bob_address, global_context);
					
			// 		return builder.processPreKey(device).catch(function(error) {
			// 			if (error.message === "Identity key changed") {
			// 				error = new textsecure.OutgoingIdentityKeyError(
			// 					number, this.message.toArrayBuffer(),
			// 					this.timestamp, device.identityKey);
			// 				this.registerError(number, "Identity key changed", error);
			// 			}
			// 			throw error;
			// 		}.bind(this));
			// 	}

			// }.bind(this)));
		// }.bind(this);
	}

	void getKeysForNumber(std::string& number, std::vector<int> updateDevices) {
		if (updateDevices.empty()) {
			auto result = m_server->getKeysForNumber(number);
			handleKeyResults(result, updateDevices);
		} else {
			for (int device : updateDevices) {
				auto result = m_server->getKeysForNumber(number, device);
				handleKeyResults(result, updateDevices);
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
		updateDevices.push_back(1);
		return updateDevices;
		// return textsecure.storage.protocol.getDeviceIds(number).then(function(deviceIds) {
		// 	if (deviceIds.length === 0) {
		// 		return [1];
		// 	}
		// 	var updateDevices = [];
		// 	return Promise.all(deviceIds.map(function(deviceId) {
		// 		var address = new libsignal.SignalProtocolAddress(number, deviceId);
		// 		var sessionCipher = new libsignal.SessionCipher(textsecure.storage.protocol, address);
		// 		return sessionCipher.hasOpenSession().then(function(hasSession) {
		// 			if (!hasSession) {
		// 				updateDevices.push(deviceId);
		// 			}
		// 		});
		// 	})).then(function() {
		// 		return updateDevices;
		// 	});
		// });
	}

	void sendToNumber(std::string& number) {
		auto updateDevices = getStaleDeviceIdsForNumber(number);

		SIGNAL_LOG_INFO << "Send to number " << number;

		getKeysForNumber(number, updateDevices);
		// reloadDevicesAndSend(number, true);
		
				// .then(this.reloadDevicesAndSend(number, true))
				// .catch(function(error) {
				// 	this.registerError(number, "Failed to retreive new device keys for number " + number, error);
				// }.bind(this));

	}

};

} // namespace signalpp

#endif // _OUTGOING_MESSAGE_H_
