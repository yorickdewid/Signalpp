#ifndef _ENV_H_
#define _ENV_H_

#include "Storage.h"

#include <signal_protocol.h>

#include <functional>

namespace signalpp {

class Env {
public:
	Env() {
		//
	}

	void onSuccess() {
		//
	}

	void onTimeout() {
		//
	}
};

} // namespace signalpp

#endif // _ENV_H_
