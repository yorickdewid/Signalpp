#ifndef _WEBSOCKET_RESOURCE_H_
#define _WEBSOCKET_RESOURCE_H_

#include "Websocket.h"
#include "WebSocketProtocol.pb.h"

#include <functional>

namespace signalpp {

struct IncomingWebSocketRequest {
	std::string verb;
	std::string path;
	std::string body;
	long long unsigned id;
	Websocket *socket;

	IncomingWebSocketRequest(std::string _verb, std::string _path, std::string _body, long long unsigned _id, Websocket *_socket)
	: verb(_verb)
	, path(_path)
	, body(_body)
	, id(_id)
	, socket(_socket) {
		
		/* Must have an id */
		if (!id) {
			// id = random 2 byte
		}

		std::cout << "IncomingWebSocketRequest:" << std::endl;
		SIGNAL_LOG_DEBUG << " verb: " << verb;
		SIGNAL_LOG_DEBUG << " path: " << path;
		SIGNAL_LOG_DEBUG << " body sz: " << body.size();
		SIGNAL_LOG_DEBUG << " id: " << id;
	}

	void respond(unsigned short status, const std::string message) {
		std::string data;

		textsecure::WebSocketResponseMessage *response = new textsecure::WebSocketResponseMessage;
		response->set_id(id);
		response->set_message(message);
		response->set_status(200);

		textsecure::WebSocketMessage wsmessage;
		wsmessage.set_type(textsecure::WebSocketMessage::RESPONSE);
		wsmessage.set_allocated_response(response);
		wsmessage.SerializeToString(&data);

		socket->send(data, true);
	}
};

struct OutgoingWebSocketRequest {

};

class WebSocketResource {
	Websocket *m_socket;
	std::function<void(IncomingWebSocketRequest request)> handleRequest;

	void onMessage(const std::string& data) {
		textsecure::WebSocketMessage message;
		message.ParseFromString(data);

        if (message.type() == textsecure::WebSocketMessage::REQUEST) {
            handleRequest(
                IncomingWebSocketRequest(
                    message.request().verb(),
                    message.request().path(),
                    message.request().body(),
                    message.request().id(),
                    m_socket
                )
            );
        } else if (message.type() == textsecure::WebSocketMessage::RESPONSE) {
        	puts("Someday not far from now this function will be shaped out of the ashes of the great, great and powerfull JS core");
            // auto response = message.response();
            // var request = outgoing[response.id()];
            // if (request) {
            //     request.response = response;
            //     var callback = request.error;
            //     if (response.status >= 200 && response.status < 300) {
            //         callback = request.success;
            //     }

            //     if (typeof callback === 'function') {
            //         callback(response.message, response.status(), request);
            //     }
            // } else {
            //     throw 'Received response for unknown request ' + response.id();
            // }
        } else {
			std::cout << "Invalid websocket request" << std::endl;
        }
	};

	void onOpen() {
		// Start keepalive timer
	}

  public:
	WebSocketResource(Websocket *socket, std::function<void(IncomingWebSocketRequest request)> callback)
	: m_socket(socket)
	, handleRequest(callback) {

		/* Call internal onMessage when hook is triggered */
		socket->onMessage([this] (const std::string& data) {
			this->onMessage(data);
		});

		socket->connect();
		// Register timer
	}

	void sendRequest() {
		// return new OutgoingWebSocketRequest(options, socket);
	}

	void close() {}
};

} // namespace signalpp

#endif // _WEBSOCKET_RESOURCE_H_
