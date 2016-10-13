#ifndef _REGISTRATION_H_
#define _REGISTRATION_H_

#include "Storage.h"

namespace signal {

namespace Registration {

static void markDone(class StorageContainer& storage) {
	storage.put("RegistrationDoneEver", "1");
	storage.put("RegistrationDone", "1");
}

static bool isDone(class StorageContainer& storage) {
	// storage.get('RegistrationDone') === "1";
	// check if RegistrationDone exists
	return false;
}

static bool everDone(class StorageContainer& storage) {
    //return storage.get('chromiumRegistrationDoneEver') === '' ||
    //       storage.get('chromiumRegistrationDone') === '';

    // check if both exist
    return false;
}

static void remove(class StorageContainer& storage) {
	//storage.purge('RegistrationDone');
}

} // namespace Registration

} // namespace signal

#endif // _REGISTRATION_H_

