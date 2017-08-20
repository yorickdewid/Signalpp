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
	std::shared_ptr<signalpp::StorageContainer> m_storage = nullptr;

public:
	MessageSender(std::shared_ptr<signalpp::StorageContainer> storage,
						const std::string& url,
						const unsigned short ports[],
						const std::string& username,
						const std::string& password,
						const std::string& signalingKey,
						const std::string& attachment_server_url)
	: m_server(new TextSecureServer(url, username, password, ports, attachment_server_url))
	, m_storage(storage) {
	}

	void sendMessageProto(long int timestamp, std::vector<std::string>& numbers, textsecure::Content& message, std::function<void()> callback) {
		std::string data;
		message.SerializeToString(&data);

		auto outgoing = new OutgoingMessage(m_server, m_storage, timestamp, numbers, data, callback);

		for (std::string& number : numbers) {
			outgoing->sendToNumber(number);
		}
	}

	long get_time_stamp() {
		std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		return ms.count();
	}

	void sendMessage(std::string& myNumber,int& deviceId, std::string* body, std::string to) {
		long timestamp = get_time_stamp();
		//structure -> Content container -> DataMessage -> Body
		textsecure::Content& proto = textsecure::Content();
		textsecure::DataMessage* message = new textsecure::DataMessage(); // gets deleted twice hmm..
		message->set_allocated_body(body);
		proto.set_allocated_datamessage(message);

		std::string* body2 = new std::string(*body);
		// we recreate it here because the Content protobuf class cleans up automagically..
		message = new textsecure::DataMessage(); // gets deleted twice hmm..
		message->set_allocated_body(body2);
		//messageSender.sendMessageProto(timestamp, numbers, proto, [] {});
		// this only sends to reciptient, but fails to sync with first device.
		sendIndividualProto(to, proto, timestamp); // we should get back the message from server and determine if syncing is neccessary..
		/* We need a sync message to sync with the first device */

		sendSyncMessage(myNumber,deviceId,message,timestamp);
	}

	void sendSyncMessage(std::string& myNumber, int& deviceId, textsecure::DataMessage* message, long int timestamp) {
		/* Primary device */
		if (deviceId == 1) {
			return;
		}
		textsecure::SyncMessage_Sent* sentSyncMessage = new textsecure::SyncMessage_Sent();
		sentSyncMessage->set_timestamp(timestamp);
		sentSyncMessage->set_allocated_message(message);
		sentSyncMessage->set_destination(myNumber);

		textsecure::SyncMessage* syncMessage = new textsecure::SyncMessage();
		syncMessage->set_allocated_sent(sentSyncMessage);

		textsecure::Content contentMessage = textsecure::Content();
		contentMessage.set_allocated_syncmessage(syncMessage);

		sendIndividualProto(myNumber, contentMessage, timestamp);
		//delete syncMessage;
		//delete sentSyncMessage;
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

		return sendIndividualProto(myNumber, contentMessage, getTimestampLong());
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

		return sendIndividualProto(myNumber, contentMessage, getTimestampLong());
	}
};

} // namespace signalpp

#endif // _MESSAGE_SENDER_H_
