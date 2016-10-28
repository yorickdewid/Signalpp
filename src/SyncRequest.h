#ifndef _SYNC_REQUEST_H_
#define _SYNC_REQUEST_H_

#if 0

#include "Storage.h"

#include <signal_protocol.h>

#include <functional>

namespace signalpp {

class SyncRequest {
	bool contactSync = false;
	bool groupSync = false;

public:
	SyncRequest(MessageSender sender, MessageReceiver receiver) {
		if (!(sender instanceof textsecure.MessageSender) || !(receiver instanceof textsecure.MessageReceiver)) {
			throw new Error('Tried to construct a SyncRequest without MessageSender and MessageReceiver');
		}
		this.receiver = receiver;

		this.oncontact = this.onContactSyncComplete.bind(this);
		receiver.addEventListener('contactsync', this.oncontact);

		this.ongroup = this.onGroupSyncComplete.bind(this);
		receiver.addEventListener('groupsync', this.ongroup);

		sender.sendRequestContactSyncMessage().then(function() {
			sender.sendRequestGroupSyncMessage();
		});
		this.timeout = setTimeout(this.onTimeout.bind(this), 60000);
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

    void update() {
        if (contactSync && groupSync) {
            // call success callback
            cleanup();
        }
    }

    void onTimeout() {
        if (contactSync || groupSync) {
            // call success callback
        } else {
            // call timeout callback
        }

        cleanup();
    }

    void cleanup() {
        clearTimeout(this.timeout);
        this.receiver.removeEventListener('contactsync', this.oncontact);
        this.receiver.removeEventListener('groupSync', this.ongroup);
        delete this.listeners;
    }
};

} // namespace signalpp

#endif

#endif // _SYNC_REQUEST_H_
