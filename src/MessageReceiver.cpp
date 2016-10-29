#include "MessageReceiver.h"
#include "WebsocketResource.h"

#include <sstream>

using namespace signalpp;

void MessageReceiver::connect() {
	SIGNAL_LOG_INFO << "Opening websocket";

	/* Initialize the socket and start listening for messages */	
	m_socket = m_server->getMessageSocket();
	
	m_socket->onOpen([] {
		SIGNAL_LOG_INFO << "MessageReceiver Websocket open";
	});

	m_socket->onFail([] {
		SIGNAL_LOG_ERROR << "MessageReceiver Websocket error";
	});

	//TODO: move into member function
	m_socket->onClose([] {
		SIGNAL_LOG_INFO << "MessageReceiver Websocket closed";
	});

	WebSocketResource(m_socket, [&] (IncomingWebSocketRequest request) {
		// We do the message decryption here, instead of in the ordered pending queue,
		// to avoid exposing the time it took us to process messages through the time-to-ack.

		// TODO: handle different types of requests. for now we blindly assume
		// PUT /messages <encrypted Envelope>
		SIGNAL_LOG_INFO << "Whoops message";
				// textsecure.crypto.decryptWebsocketMessage(request.body, this.signalingKey).then(function(plaintext) {
				// 	var envelope = textsecure.protobuf.Envelope.decode(plaintext);
				// 	// After this point, decoding errors are not the server's
				// 	// fault, and we should handle them gracefully and tell the
				// 	// user they received an invalid message
				// 	request.respond(200, 'OK');

				// 	if (!this.isBlocked(envelope.source)) {
				// 		this.queueEnvelope(envelope);
				// 	}

				// }.bind(this)).catch(function(e) {
				// 	request.respond(500, 'Bad encrypted websocket message');
				// 	console.log("Error handling incoming message:", e);
				// 	var ev = new Event('error');
				// 	ev.error = e;
				// 	this.dispatchEvent(ev);
				// }.bind(this));
	});
}
