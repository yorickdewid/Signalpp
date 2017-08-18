#ifndef _ENV_H_
#define _ENV_H_

#include "Logger.h"
#include "Storage.h"
#include "Registration.h"

#include <signal_protocol.h>

#include <functional>

namespace signalpp {

class Env {
	std::shared_ptr<signalpp::StorageContainer> m_storage = nullptr;

	void init() {
		std::cout << "Initialize environment" << std::endl;

		if (!signalpp::Registration::isDone(m_storage)) {
			m_storage->put("startcount", 0);

			m_storage->put("username", "");
			m_storage->put("password", "");

			m_storage->put("maxPreKeyId", 1);
			m_storage->put("signedKeyId", 1);
		}
	}

public:
	Env(std::shared_ptr<signalpp::StorageContainer> storage)
	: m_storage(storage) {
		init();
	}

	static void purge(std::shared_ptr<signalpp::StorageContainer> storage) {
		signalpp::Registration::remove(storage);

		storage->purge("identityKey");
		storage->purge("signaling_key");
		storage->purge("password");
		storage->purge("registrationId");
		storage->purge("number");
		storage->purge("number_id");
		storage->purge("device_name");
		storage->purge("device_id");
		storage->purge("userAgent");
	}
};

} // namespace signalpp

#endif // _ENV_H_
