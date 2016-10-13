#ifndef _WEBSOCKET_RESOURCE_H_
#define _WEBSOCKET_RESOURCE_H_

namespace signal {

struct IncomingWebSocketRequest {
	IncomingWebSocketRequest(std::string verb,
							std::string path,
							std::string body,
							int id
							//io_service m_socket)
							)
	: m_verb(verb)
	, m_path(path)
	, m_body(body)
	, m_id(id)
	//, m_socket(m_socket) {
	{
		
		/* Must have an id */
		if (id == 0) {
			// id = random 2 byte
		}

	}

	void respond() {
		// send via m_socket
	}

  private:
	std::string m_verb;
	std::string m_path;
	std::string m_body;
	int m_id;
	// io_service m_socket;
};

struct OutgoingWebSocketRequest {

};

class WebSocketResource {

	void onMessage() {
		// Decode using protobuf to message

		// if (message.type === textsecure.protobuf.WebSocketMessage.Type.REQUEST ) {
		//     call lambda( new IncomingWebSocketRequest({
        //                    message.request.verb,
        //                    message.request.path,
        //                    message.request.body,
        //                    message.request.id,
        //                    socket
        //                }) )
		// else
		//     something else
	}

	void onOpen() {
		// Start keepalive timer
	}

  public:
  	WebSocketResource(/* var socket, callback lambda */) {
  		// Websocket is already active at this point
  		
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
