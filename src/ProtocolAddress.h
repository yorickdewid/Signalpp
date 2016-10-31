#ifndef _PROTOCOL_ADDRESS_H_
#define _PROTOCOL_ADDRESS_H_

#include <signal_protocol.h>

namespace signalpp {

struct ProtocolAddress {
	signal_protocol_address m_address;

	ProtocolAddress(std::string number, int deviceId) {
		m_address.name = number.c_str();
		m_address.name_len = number.size();
		m_address.device_id = deviceId;
	}

};

} // namespace signalpp

#endif // _PROTOCOL_ADDRESS_H_
