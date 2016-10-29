#ifndef _MESSAGE_RECEIVER_H_
#define _MESSAGE_RECEIVER_H_

#include "Api.h"
#include "Storage.h"

#include <signal_protocol.h>

namespace signalpp {

class MessageReceiver {
	std::unique_ptr<TextSecureServer> m_server;
	StorageContainer *m_storage = nullptr;
	Websocket *m_socket = nullptr;

	void connect();

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
		connect();
	}

	~MessageReceiver() {
		delete m_socket;
	}
};

} // namespace signalpp

#endif // _MESSAGE_RECEIVER_H_
