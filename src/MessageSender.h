#ifndef _MESSAGE_SENDER_H_
#define _MESSAGE_SENDER_H_

#include "Api.h"
#include "Storage.h"
#include "OutgoingMessage.h"

#include "IncomingPushMessageSignal.pb.h"

#include <signal_protocol.h>

namespace signalpp {

class MessageSender {
	std::shared_ptr<TextSecureServer> m_server;
	StorageContainer *m_storage = nullptr;

public:
	MessageSender(class StorageContainer *storage,
						const std::string& url,
						const unsigned short ports[],
						const std::string& username,
						const std::string& password,
						const std::string& signalingKey,
						const std::string& attachment_server_url)
	: m_server(new TextSecureServer(url, username, password, ports, attachment_server_url))
	, m_storage(storage) {
		// this.url = url;
		// this.signalingKey = signalingKey;
		// this.username = username;
		// this.password = password;

		// var address = libsignal.SignalProtocolAddress.fromString(username);
		// this.number = address.getName();
		// this.deviceId = address.getDeviceId();
	}

	void sendMessageProto(long int timestamp, std::vector<std::string>& numbers, textsecure::Content& message, std::function<void()> callback) {
		std::string data;
		message.SerializeToString(&data);

		auto outgoing = new OutgoingMessage(m_server, timestamp, numbers, data, callback);

		for (std::string& number : numbers) {
			outgoing->sendToNumber(number);
		}
	}

	void sendIndividualProto(std::string& number, textsecure::Content& proto, long int timestamp) {
		std::vector<std::string> numbers;
		numbers.push_back(number);

		sendMessageProto(timestamp, numbers, proto, [] {
			// if (res.errors.length > 0)
			// 	reject(res);
			// else
			// 	resolve(res);
		});
	}

	void sendRequestGroupSyncMessage() {
		std::string myNumber;
		if (!m_storage->get("number", myNumber))//TOOD: user User storage layer
			return;

		int myDevice = 0;
		if (!m_storage->get("device_id", myDevice))
			return;

		if (!myDevice || myDevice == 1)
			return;

		textsecure::SyncMessage_Request *request = new textsecure::SyncMessage_Request;
		request->set_type(textsecure::SyncMessage_Request::GROUPS);

		textsecure::SyncMessage *syncMessage = new textsecure::SyncMessage;
		syncMessage->set_allocated_request(request);

		textsecure::Content contentMessage;
		contentMessage.set_allocated_syncmessage(syncMessage);

		return sendIndividualProto(myNumber, contentMessage, getTimestamp());
	}

	void sendRequestContactSyncMessage() {
		std::string myNumber;
		if (!m_storage->get("number", myNumber))//TOOD: user User storage layer
			return;

		int myDevice = 0;
		if (!m_storage->get("device_id", myDevice))
			return;

		if (!myDevice || myDevice == 1)
			return;

		textsecure::SyncMessage_Request *request = new textsecure::SyncMessage_Request;
		request->set_type(textsecure::SyncMessage_Request::CONTACTS);

		textsecure::SyncMessage *syncMessage = new textsecure::SyncMessage;
		syncMessage->set_allocated_request(request);

		textsecure::Content contentMessage;
		contentMessage.set_allocated_syncmessage(syncMessage);

		return sendIndividualProto(myNumber, contentMessage, getTimestamp());
	}
};

} // namespace signalpp

#endif // _MESSAGE_SENDER_H_
