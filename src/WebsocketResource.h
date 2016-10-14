#ifndef _WEBSOCKET_RESOURCE_H_
#define _WEBSOCKET_RESOURCE_H_

#include <functional>

namespace signal {

struct IncomingWebSocketRequest {
	std::string verb;
	std::string path;
	std::string body;
	int id;
	int socket;

	IncomingWebSocketRequest(std::string _verb, std::string _path, std::string _body, int _id, int _socket)
	: verb(_verb)
	, path(_path)
	, body(_body)
	, id(_id)
	, socket(_socket) {
		
		/* Must have an id */
		if (id == 0) {
			// id = random 2 byte
		}
	}

	void respond(unsigned short status, const std::string& message) {
		// socket.send(
		//	new textsecure.protobuf.WebSocketMessage({
		//		type: textsecure.protobuf.WebSocketMessage.Type.RESPONSE,
		//		response: { id: id, message: message, status: status }
		//		}).encode().toArrayBuffer()
		//	);
	}
};

struct OutgoingWebSocketRequest {

};

class WebSocketResource {

	int m_socket;
	std::function<void(IncomingWebSocketRequest request)> handleRequest;

	void onMessage() {
		// Decode using protobuf to message

		// if (message.type === textsecure.protobuf.WebSocketMessage.Type.REQUEST ) {
		handleRequest(IncomingWebSocketRequest(
			"PUT",
			//message.request.verb,
			"/v1/address",
			//message.request.path,
			"",
			//message.request.body,
			7384653,
			//message.request.id,
			m_socket
		));
		// else
		//     something else
	}

	void onOpen() {
		// Start keepalive timer
	}

  public:
  	WebSocketResource(int socket, std::function<void(IncomingWebSocketRequest request)> callback) : m_socket(socket), handleRequest(callback) {
  		// Websocket is already active at this point
  		
  		onMessage();
  		// Register lambda
  		// Register Websocket hanlers
  		// Register timer
	}

  	void sendRequest() {
  		// return new OutgoingWebSocketRequest(options, socket);
  	}

  	void close() {}
};

} // namespace signal

#endif // _WEBSOCKET_RESOURCE_H_
