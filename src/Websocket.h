#ifndef _WEBSOCKET_H_
#define _WEBSOCKET_H_

#include "Logger.h"

#include <libwebsockets.h>

namespace signalpp {

static const int ietf_version = -1;

enum class WebsocketStatus {
	CONNECT,
	OPEN,
	FAILED,
	CLOSED,
};

/*class WebsocketHandler {
protected:
	WebsocketStatus m_status;

public:
	WebsocketHandler() : m_status(WebsocketStatus::CONNECT) {}

	virtual void onOpen() = 0;
	virtual void onFail() = 0;
	virtual void onClose() = 0;
	virtual void onMessage() = 0;

	WebsocketStatus status() const {
		return m_status;
	}
};*/

class Websocket {
	struct lws_client_connect_info m_conn_info;
	struct lws_context_creation_info m_info;
	struct lws_context *m_context = nullptr;
	struct lws *m_wsi = nullptr;
	bool m_ssl = false;
	WebsocketStatus m_status;

public:
	Websocket(const std::string& uri);
	~Websocket();

	bool connect();

	void setStatus(WebsocketStatus status) {
		m_status = status;
	}

	void close(int code, std::string reason) {}

	void send(std::string message) {}
};

} // namespace signalpp

#endif // _WEBSOCKET_H_
