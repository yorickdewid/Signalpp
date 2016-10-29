#ifndef _MESSAGE_SENDER_H_
#define _MESSAGE_SENDER_H_

#include "Api.h"
#include "Storage.h"

#include <signal_protocol.h>

namespace signalpp {

class MessageSender {
	std::unique_ptr<TextSecureServer> m_server;
	StorageContainer *m_storage = nullptr;

	void connect();

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

	void sendIndividualProto(std::string& number, void *proto, long int timestamp) {
		// return new Promise(function(resolve, reject) {
		// 	this.sendMessageProto(timestamp, [number], proto, function(res) {
		// 		if (res.errors.length > 0)
		// 			reject(res);
		// 		else
		// 			resolve(res);
		// 	});
		// }.bind(this));
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

		// var request = new textsecure.protobuf.SyncMessage.Request();
		// request.type = textsecure.protobuf.SyncMessage.Request.Type.GROUPS;
		// var syncMessage = new textsecure.protobuf.SyncMessage();
		// syncMessage.request = request;
		// var contentMessage = new textsecure.protobuf.Content();
		// contentMessage.syncMessage = syncMessage;

		// return sendIndividualProto(myNumber, contentMessage, Date.now());
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

		// var request = new textsecure.protobuf.SyncMessage.Request();
		// request.type = textsecure.protobuf.SyncMessage.Request.Type.CONTACTS;
		// var syncMessage = new textsecure.protobuf.SyncMessage();
		// syncMessage.request = request;
		// var contentMessage = new textsecure.protobuf.Content();
		// contentMessage.syncMessage = syncMessage;

		// return sendIndividualProto(myNumber, contentMessage, getTimestamp());
	}
};

} // namespace signalpp

#endif // _MESSAGE_SENDER_H_
