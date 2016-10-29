#ifndef _SYNC_REQUEST_H_
#define _SYNC_REQUEST_H_

#include "Storage.h"

#include <signal_protocol.h>

#include <functional>

namespace signalpp {

class SyncRequest {
	bool contactSync = false;
	bool groupSync = false;

	void update() {
		if (contactSync && groupSync) {
			// call success callback
			cleanup();
		}
	}

	// void onTimeout() {
	// 	if (contactSync || groupSync) {
	// 		// call success callback
	// 	} else {
	// 		// call timeout callback
	// 	}

	// 	cleanup();
	// }

	void cleanup() {
		// clearTimeout(this.timeout);
		// this.receiver.removeEventListener('contactsync', this.oncontact);
		// this.receiver.removeEventListener('groupSync', this.ongroup);
		// delete this.listeners;
	}

public:
	SyncRequest(MessageSender& sender, MessageReceiver& receiver) {
		// this.receiver = receiver;

		sender.sendRequestContactSyncMessage();
		sender.sendRequestGroupSyncMessage();
		// this.timeout = setTimeout(this.onTimeout.bind(this), 60000);
	}

	void onSuccess(std::function<void()> callback) {
		//register success callback
	}

	void onTimeout(std::function<void()> callback) {
		//register timeout callback
	}

	void onContactSyncComplete() {
		contactSync = true;
		update();
	}

	void onGroupSyncComplete() {
		groupSync = true;
		update();
	}
};

} // namespace signalpp

#endif // _SYNC_REQUEST_H_
