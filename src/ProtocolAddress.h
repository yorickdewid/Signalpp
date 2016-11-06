#ifndef _PROTOCOL_ADDRESS_H_
#define _PROTOCOL_ADDRESS_H_

#include <signal_protocol.h>

namespace signalpp {

struct ProtocolAddress {
	signal_protocol_address address;
	char *_name = nullptr;

	ProtocolAddress(std::string number, int deviceId) {
		address.name = strdup(number.c_str());
		address.name_len = number.size();
		address.device_id = deviceId;
	}

	~ProtocolAddress() {
		free((void *)address.name);
	}

	std::string getName() const {
		return address.name;
	}
	
	int getDeviceId() const {
		return address.device_id;
	}
	
	std::string toString() const {
		return std::string(address.name) + "." + std::to_string(address.device_id);
	}

	bool operator== (const ProtocolAddress &rhs) {
		//TODO: use strcmp
		return address.device_id == rhs.address.device_id &&
				address.name_len == rhs.address.name_len &&
				!memcmp(address.name, rhs.address.name, rhs.address.name_len);
	}

	friend std::ostream& operator<< (std::ostream& os, const ProtocolAddress& rhs) {
		os << rhs.address.name << ":" << rhs.address.device_id;
		return os;
	}
};

} // namespace signalpp

#endif // _PROTOCOL_ADDRESS_H_
