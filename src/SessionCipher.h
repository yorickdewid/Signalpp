#ifndef _SESSION_CIPHER_H_
#define _SESSION_CIPHER_H_

#include "CryptoProvider.h"
#include "ProtocolAddress.h"
#include "ProtocolStore.h"

#include <signal_protocol.h>

namespace signalpp {

class SessionCipher {
	ProtocolAddress m_address;

public:
	SessionCipher(ProtocolAddress& address) : m_address(address) {
		//
	}

	~SessionCipher() {
		//
	}
};

} // namespace signalpp

#endif // _SESSION_CIPHER_H_
