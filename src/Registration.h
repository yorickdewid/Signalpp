#ifndef _REGISTRATION_H_
#define _REGISTRATION_H_

#include "Storage.h"

namespace signalpp {

namespace Registration {

static void markDone(std::shared_ptr<signalpp::StorageContainer> storage) {
	storage->put("RegistrationDoneEver", 1);
	storage->put("RegistrationDone", 1);
}

static bool isDone(std::shared_ptr<signalpp::StorageContainer> storage) {
	int _ph = 0;
	char buffer[30];
	bool result = storage->get("RegistrationDone", _ph);
	sprintf(buffer, "length: %i", _ph);
	printf_s(buffer);
	return result;
}

static bool everDone(std::shared_ptr<signalpp::StorageContainer> storage) {
	int _ph = 0;
	return storage->get("RegistrationDoneEver", _ph)
		&& storage->get("RegistrationDone", _ph);
}

static void remove(std::shared_ptr<signalpp::StorageContainer> storage) {
	storage->purge("RegistrationDoneEver");
	storage->purge("RegistrationDone");
}

} // namespace Registration

} // namespace signalpp

#endif // _REGISTRATION_H_

