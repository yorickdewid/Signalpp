#ifndef _MESSAGE_RECEIVER_H_
#define _MESSAGE_RECEIVER_H_

#include "Storage.h"

#include <signal_protocol.h>

namespace signalpp {

class MessageReceiver {
	std::unique_ptr<TextSecureServer> m_server;
	StorageContainer *m_storage = nullptr;

	void connect() {
		// if (this.socket && this.socket.readyState !== WebSocket.CLOSED) {
		// 	this.socket.close();
		// }

		SIGNAL_LOG_INFO << "Opening websocket";

		/* Initialize the socket and start listening for messages */
		auto socket = m_server->getMessageSocket();
		
		// this.socket.onclose = this.onclose.bind(this);
		// this.socket.onerror = this.onerror.bind(this);
		// this.socket.onopen = this.onopen.bind(this);
		// this.wsr = new WebSocketResource(this.socket, {
		// 	handleRequest: this.handleRequest.bind(this),
		// 	keepalive: { path: '/v1/keepalive', disconnect: true }
		// });
		// this.pending = Promise.resolve();

		delete socket;
	}

public:
	MessageReceiver(class StorageContainer *storage,
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
};

} // namespace signalpp

#endif // _MESSAGE_RECEIVER_H_
