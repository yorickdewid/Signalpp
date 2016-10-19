#ifndef _WEBSOCKET_H_
#define _WEBSOCKET_H_

#include "Logger.h"

#include <functional>

#include <libwebsockets.h>

namespace signalpp {

constexpr int ietf_version = -1;

enum class WebsocketStatus {
	CONNECT,
	OPEN,
	FAILED,
	CLOSED,
};

class Websocket {
	struct lws_client_connect_info m_conn_info;
	struct lws_context_creation_info m_info;
	struct lws_context *m_context = nullptr;
	struct lws *m_wsi = nullptr;
	bool m_ssl = false;
	int m_conn_timeout = 60;
	WebsocketStatus m_status;

	/* Callbacks */
	std::function<void(const std::string message)> m_onMessageCallback = nullptr;
	std::function<void()> m_onCloseCallback = nullptr;
	std::function<void()> m_onOpenCallback = nullptr;
	std::function<void()> m_onFailCallback = nullptr;

public:
	Websocket(const std::string& uri);
	~Websocket();

	void connect();

	void setStatus(WebsocketStatus status) {
		m_status = status;
	}

	/* Call registered routines */
	inline void callMessage(const std::string& message) {
		if (!m_onMessageCallback)
			return;

		m_onMessageCallback(message);
	}

	inline void callClose() {
		if (!m_onCloseCallback)
			return;

		m_onCloseCallback();
	}

	inline void callOpen() {
		if (!m_onOpenCallback)
			return;

		m_onOpenCallback();
	}

	inline void callFail() {
		if (!m_onFailCallback)
			return;

		m_onFailCallback();
	}

	/* Register callbacks */
	void onMessage(std::function<void(const std::string&)> onMessageCallback) {
		m_onMessageCallback = onMessageCallback;
	}
	
	void onClose(std::function<void()> onCloseCallback) {
		m_onCloseCallback = onCloseCallback;
	}

	void onOpen(std::function<void()> onOpenCallback) {
		m_onOpenCallback = onOpenCallback;
	}

	void onFail(std::function<void()> onFailCallback) {
		m_onFailCallback = onFailCallback;
	}

	void ping() {
		unsigned char buf[LWS_PRE];
		lws_write(m_wsi, &buf[LWS_PRE], 0, LWS_WRITE_PING);
	}

	void close(enum lws_close_status status = LWS_CLOSE_STATUS_NORMAL, std::string reason = "") {
		lws_close_reason(m_wsi, status, (unsigned char *)reason.c_str(), reason.size());
	}

	void send(std::string message, bool binary = false) {
		unsigned char *buf = (unsigned char *)malloc(LWS_PRE + message.size());
		memcpy(&buf[LWS_PRE], message.c_str(), message.size());
		lws_write(m_wsi, &buf[LWS_PRE], message.size(), binary ? LWS_WRITE_BINARY : LWS_WRITE_TEXT);
		free(buf);
	}
};

} // namespace signalpp

#endif // _WEBSOCKET_H_
