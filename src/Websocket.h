#ifndef _WEBSOCKET_H_
#define _WEBSOCKET_H_

#include "Logger.h"

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <sstream>

namespace signalpp {

typedef websocketpp::client<websocketpp::config::asio_client> client;

enum class WebsocketStatus {
	CONNECT,
	OPEN,
	FAILED,
	CLOSED,
};

class WebsocketHandler {
	websocketpp::connection_hdl m_hdl;

protected:
	WebsocketStatus m_status;

public:
	WebsocketHandler(websocketpp::connection_hdl hdl) : m_hdl(hdl), m_status(WebsocketStatus::CONNECT) {}

	virtual void onOpen(client *c, websocketpp::connection_hdl hdl) = 0;
	virtual void onFail(client *c, websocketpp::connection_hdl hdl) = 0;
	virtual void onClose(client *c, websocketpp::connection_hdl hdl) = 0;
	virtual void onMessage(websocketpp::connection_hdl, client::message_ptr msg) = 0;

	websocketpp::connection_hdl handle() const {
		return m_hdl;
	}

	WebsocketStatus status() const {
		return m_status;
	}
};

template <class type>
class Websocket {
	client m_endpoint;
	websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;
	websocketpp::lib::shared_ptr<type> m_connection;

public:
	Websocket () {
		//m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
		//m_endpoint.clear_error_channels(websocketpp::log::elevel::all);

		m_endpoint.init_asio();
		m_endpoint.start_perpetual();

		m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client::run, &m_endpoint);
	}

	~Websocket() {
		m_endpoint.stop_perpetual();
		
		/* Close connection if still open */
		if (m_connection->status() != WebsocketStatus::OPEN)
			return;
		
		SIGNAL_LOG_INFO << "Websocket closing connection";
		
		websocketpp::lib::error_code ec;
		m_endpoint.close(m_connection->handle(), websocketpp::close::status::going_away, "", ec);
		if (ec) {
			SIGNAL_LOG_ERROR << "Websocket error closing connection: " << ec.message();
		}
		
		m_thread->join();
	}

	bool connect(const std::string& uri) {
		websocketpp::lib::error_code ec;

		client::connection_ptr connection = m_endpoint.get_connection(uri, ec);

		if (ec) {
			SIGNAL_LOG_ERROR << "Websocket connect initialization error: " << ec.message();
			return false;
		}

		m_connection = websocketpp::lib::make_shared<type>(connection->get_handle(), uri);

		connection->set_open_handler(websocketpp::lib::bind(
			&type::onOpen,
			m_connection,
			&m_endpoint,
			websocketpp::lib::placeholders::_1
		));

		connection->set_fail_handler(websocketpp::lib::bind(
			&type::onFail,
			m_connection,
			&m_endpoint,
			websocketpp::lib::placeholders::_1
		));

		connection->set_close_handler(websocketpp::lib::bind(
			&type::onClose,
			m_connection,
			&m_endpoint,
			websocketpp::lib::placeholders::_1
		));

		connection->set_message_handler(websocketpp::lib::bind(
			&type::onMessage,
			m_connection,
			websocketpp::lib::placeholders::_1,
			websocketpp::lib::placeholders::_2
		));

		m_endpoint.connect(connection);

		return true;
	}

	void close(websocketpp::close::status::value code, std::string reason) {
		websocketpp::lib::error_code ec;
		
		m_endpoint.close(m_connection->handle(), code, reason, ec);
		if (ec) {
			SIGNAL_LOG_ERROR << "Websocket error initiating close: " << ec.message();
		}
	}

	void send(std::string message) {
		websocketpp::lib::error_code ec;
		
		m_endpoint.send(m_connection->handle(), message, websocketpp::frame::opcode::text, ec);
		if (ec) {
			SIGNAL_LOG_ERROR << "Websocket error sending message: " << ec.message();
			return;
		}
	}

	websocketpp::lib::shared_ptr<type> instance() const {
		return m_connection;
	}
};

} // namespace signalpp

#endif // _WEBSOCKET_H_
