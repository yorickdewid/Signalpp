#include "Websocket.h"

using namespace signalpp;

int c_callback(struct lws *wsi, enum lws_callback_reasons reason,
			void *user, void *in, size_t len) {

	Websocket *ws = nullptr;
	if (lws_get_protocol(wsi)) {
		ws = (Websocket *)lws_get_protocol(wsi)->user;

		// ws->printStatus();
	}

	switch (reason) {
		case LWS_CALLBACK_CLIENT_ESTABLISHED:
			if (ws) {
				//SIGNAL_LOG_DEBUG << "Call onOpen";
				std::cout << "Call onOpen" << std::endl;
				ws->callOpen();
			}
			break;

		case LWS_CALLBACK_CLOSED:
			if (ws) {
				//SIGNAL_LOG_DEBUG << "Call onClose";
				std::cout << "Call onClose" << std::endl;
				ws->callClose();
			}
			break;

		case LWS_CALLBACK_CLIENT_RECEIVE: {
			if (ws) {
				//SIGNAL_LOG_DEBUG << "Call onMessage";
				std::cout << "Call onMessage" << std::endl;
				ws->callMessage(std::string((char *)in, len));
			}
			break;
		}

		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
			if (ws) {
				//SIGNAL_LOG_DEBUG << "Call onFail";
				std::cout << "Call onFail" << std::endl;
				ws->callFail();
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

static struct lws_protocols protocols[] = {
	{
		"c_callback",
		c_callback,
		0,
		256,
	},
	{ NULL, NULL, 0, 0 } /* end */
};


static const struct lws_extension exts[] = {
	{ "permessage-deflate", lws_extension_callback_pm_deflate, "permessage-deflate; client_max_window_bits" },
	{ "deflate-frame", lws_extension_callback_pm_deflate, "deflate_frame" },
	{ NULL, NULL, NULL /* terminator */ }
};

Websocket::Websocket(const std::string& uri) {
	const char *_proto, *_path, *_address;

	m_status = WebsocketStatus::CLOSED;

	memset(&m_info, 0, sizeof(m_info));
	memset(&m_conn_info, 0, sizeof(m_conn_info));

	/* Parse URI */
	if (lws_parse_uri((char *)uri.c_str(), &_proto, &_address, &m_conn_info.port, &_path)) {
		// SIGNAL_LOG_ERROR << "Parsing uri error";
		std::cerr << "Parsing uri error" << std::endl;
		return;
	}

	/* Ship the class instance */
	protocols[0].user = this;

	/* Address */
	char *address = (char *)calloc(strlen(_address) + 1, sizeof(char));
	strcpy(address, _address);
	m_conn_info.address = address;

	/* Find path */
	char *path = (char *)calloc(strlen(_path) + 2, sizeof(char));
	path[0] = '/';
	strcpy(&path[1], _path);
	m_conn_info.path = path;

	/* Set secure flag if requested */
	if (!strcmp(_proto, "http") || !strcmp(_proto, "ws"))
		m_ssl = false;
	if (!strcmp(_proto, "https") || !strcmp(_proto, "wss"))
		m_ssl = true;

	m_info.port = CONTEXT_PORT_NO_LISTEN;
	m_info.protocols = protocols;
	m_info.gid = -1;
	m_info.uid = -1;

	m_context = lws_create_context(&m_info);
	if (!m_context) {
		//SIGNAL_LOG_ERROR << "Creating libwebsocket context failed";
		std::cerr << "Creating libwebsocket context failed" << std::endl;
		return;
	}

	m_conn_info.context = m_context;
	m_conn_info.ssl_connection = m_ssl;
	m_conn_info.host = m_conn_info.address;
	m_conn_info.origin = m_conn_info.address;
	m_conn_info.ietf_version_or_minus_one = ietf_version;
	m_conn_info.client_exts = exts;

	std::cout << " Address: " << m_conn_info.address << std::endl;
	std::cout << " Port: " << m_conn_info.port << std::endl;
	std::cout << " Path: " << m_conn_info.path << std::endl;
	std::cout << " SSL: " << m_ssl << std::endl;
}

Websocket::~Websocket() {
	std::cout << "Cleanup" << std::endl;

	free((void *)m_conn_info.address);
	free((void *)m_conn_info.path);

	if (m_status == WebsocketStatus::OPEN) {
		std::cout << "Websocket still open" << std::endl;
		close();
	}

	lws_context_destroy(m_context);
}

void Websocket::connect() {
	while (m_status == WebsocketStatus::CONNECT || m_status == WebsocketStatus::OPEN || !m_wsi) {
		if (!m_wsi) {
			std::cout << "Connecting ..." << std::endl;
			m_conn_info.protocol = protocols[0].name;
			m_wsi = lws_client_connect_via_info(&m_conn_info);
			if (m_wsi)
				m_status = WebsocketStatus::CONNECT;
		}

		lws_service(m_context, m_conn_timeout);
	}

	m_wsi = nullptr;
}
