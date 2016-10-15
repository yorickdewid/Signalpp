#include "Logger.h"
#include "Websocket.h"

using namespace signalpp;

class DummyHandler : public WebsocketHandler {
	std::string m_uri;

public:
	DummyHandler(websocketpp::connection_hdl hdl, std::string uri)
	: WebsocketHandler(hdl)
	, m_uri(uri) {
	}

	void onOpen(client* c, websocketpp::connection_hdl hdl) {
		m_status = WebsocketStatus::OPEN;

		client::connection_ptr con = c->get_con_from_hdl(hdl);

		SIGNAL_LOG_INFO << "Websocket opened";
		SIGNAL_LOG_DEBUG << "Websocket server: " << con->get_response_header("Server");
	}

	void onFail(client* c, websocketpp::connection_hdl hdl) {
		m_status = WebsocketStatus::FAILED;

		client::connection_ptr con = c->get_con_from_hdl(hdl);

		SIGNAL_LOG_ERROR << "Websocket failed: " << con->get_ec().message();
		SIGNAL_LOG_DEBUG << "Websocket server: " << con->get_response_header("Server");
	}
	
	void onClose(client* c, websocketpp::connection_hdl hdl) {
		m_status = WebsocketStatus::CLOSED;
		
		client::connection_ptr con = c->get_con_from_hdl(hdl);
		
		std::stringstream s;
		s << "close code: " << con->get_remote_close_code() << " (" 
		  << websocketpp::close::status::get_string(con->get_remote_close_code()) 
		  << "), close reason: " << con->get_remote_close_reason();

		SIGNAL_LOG_INFO << "Websocket closed: " << s.str();
	}

	void onMessage(websocketpp::connection_hdl, client::message_ptr msg) {
		if (msg->get_opcode() == websocketpp::frame::opcode::text) {
			SIGNAL_LOG_DEBUG << "Websocket message: " << msg->get_payload();
		} else {
			SIGNAL_LOG_DEBUG << "Websocket frame: " << websocketpp::utility::to_hex(msg->get_payload());
		}
	}

	void printStatus() {
		SIGNAL_LOG_INFO << "Websocket URI: " << m_uri;
	}
};
