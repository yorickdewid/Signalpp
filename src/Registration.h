#ifndef _REGISTRATION_H_
#define _REGISTRATION_H_

#include "Storage.h"

namespace signalpp {

namespace Registration {

static void markDone(StorageContainer& storage) {
	storage.put("RegistrationDoneEver", 1);
	storage.put("RegistrationDone", 1);
}

static bool isDone(StorageContainer& storage) {
	int _ph = 0;
	return storage.get("RegistrationDone", _ph);
}

static bool everDone(StorageContainer& storage) {
	int _ph = 0;
	return storage.get("RegistrationDoneEver", _ph)
		&& storage.get("RegistrationDone", _ph);
}

static void remove(StorageContainer& storage) {
	storage.purge("RegistrationDoneEver");
	storage.purge("RegistrationDone");
}

} // namespace Registration

} // namespace signalpp

#endif // _REGISTRATION_H_

