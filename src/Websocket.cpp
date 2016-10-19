#include "Websocket.h"

using namespace signalpp;

int c_callback(struct lws *wsi, enum lws_callback_reasons reason,
			void *user, void *in, size_t len) {

	Websocket *ws = nullptr;
	if (lws_get_protocol(wsi))
		ws = (Websocket *)lws_get_protocol(wsi)->user;

	switch (reason) {
		case LWS_CALLBACK_CLIENT_ESTABLISHED:
			if (ws) {
				ws->setStatus(WebsocketStatus::OPEN);
				ws->callOpen();
				SIGNAL_LOG_DEBUG << "Call onOpen";
			}
			break;

		case LWS_CALLBACK_CLOSED:
			if (ws) {
				ws->setStatus(WebsocketStatus::CLOSED);
				ws->callClose();
				SIGNAL_LOG_DEBUG << "Call onClose";
			}
			break;

		case LWS_CALLBACK_CLIENT_RECEIVE: {
			if (ws) {
				ws->callMessage(std::string((char *)in, len));
				SIGNAL_LOG_DEBUG << "Call onMessage";
			}
			break;
		}

		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
			if (ws) {
				ws->setStatus(WebsocketStatus::FAILED);
				ws->callFail();
				SIGNAL_LOG_DEBUG << "Call onFail";
			}
			break;

		case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
			if (!strcmp((char *)in, "deflate-stream")) {
				SIGNAL_LOG_WARNING << "denied deflate-stream extension";
				return 1;
			}
			if (!strcmp((char *)in, "x-webkit-deflate-frame"))
				return 1;
			if (!strcmp((char *)in, "deflate-frame"))
				return 1;
			break;

		default:
			break;
	}

	return 0;
}

static struct lws_protocols protocols[] = {{
		.name = "c_callback",
		.callback = &c_callback,
		.per_session_data_size = 0,
		.rx_buffer_size = 256,
		.id = 0,
		.user = nullptr,
	},

	{ nullptr, nullptr, 0, 0 } /* terminator */
};

static const struct lws_extension exts[] = {
	{ "permessage-deflate", lws_extension_callback_pm_deflate, "permessage-deflate; client_max_window_bits" },
	{ "deflate-frame", lws_extension_callback_pm_deflate, "deflate_frame" },
	{ NULL, NULL, NULL /* terminator */ }
};

Websocket::Websocket(const std::string& uri) {
	char path[312];
	const char *proto, *p;

	m_status = WebsocketStatus::CLOSED;

	memset(&m_info, 0, sizeof(m_info));
	memset(&m_conn_info, 0, sizeof(m_conn_info));

	if (lws_parse_uri((char *)uri.c_str(), &proto, &m_conn_info.address, &m_conn_info.port, &p)) {
		SIGNAL_LOG_ERROR << "Parsing uri error";
		return;
	}

	/* Ship the class instance */
	protocols[0].user = this;

	/* Find path */
	path[0] = '/';
	strncpy(path + 1, p, sizeof(path) - 2);
	path[sizeof(path) - 1] = '\0';
	m_conn_info.path = path;

	if (!strcmp(proto, "http") || !strcmp(proto, "ws"))
		m_ssl = false;
	if (!strcmp(proto, "https") || !strcmp(proto, "wss"))
		m_ssl = true;

	m_info.port = CONTEXT_PORT_NO_LISTEN;
	m_info.protocols = protocols;
	m_info.gid = -1;
	m_info.uid = -1;

	m_context = lws_create_context(&m_info);
	if (!m_context) {
		SIGNAL_LOG_ERROR << "Creating libwebsocket context failed";
		return;
	}

	m_conn_info.context = m_context;
	m_conn_info.ssl_connection = m_ssl;
	m_conn_info.host = m_conn_info.address;
	m_conn_info.origin = m_conn_info.address;
	m_conn_info.ietf_version_or_minus_one = ietf_version;
	m_conn_info.client_exts = exts;

	SIGNAL_LOG_DEBUG << " Address: " << m_conn_info.address;
	SIGNAL_LOG_DEBUG << " Port: " << m_conn_info.port;
	SIGNAL_LOG_DEBUG << " Path: " << m_conn_info.path;
	SIGNAL_LOG_DEBUG << " SSL: " << m_ssl;
}

Websocket::~Websocket() {
	SIGNAL_LOG_INFO << "Websocket closing connection";

	if (m_status == WebsocketStatus::OPEN) {
		SIGNAL_LOG_DEBUG << "Websocket still open";
		close();
	}

	lws_context_destroy(m_context);
}

void Websocket::connect() {
	while (true) {

		if (!m_wsi) {
			SIGNAL_LOG_DEBUG << "Connecting ...";
			m_conn_info.protocol = protocols[0].name;
			m_wsi = lws_client_connect_via_info(&m_conn_info);

			m_status = WebsocketStatus::CONNECT;
		}

		lws_service(m_context, m_conn_timeout);
	}
}
