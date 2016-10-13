#ifndef _API_H_
#define _API_H_

#include <vector>

namespace sinl {

class PortManager {
	unsigned int index = 0;
	std::vector<unsigned short> ports;

  public:
	void add(unsigned short port) {
		ports.push_back(port);
	}

	unsigned short getPort() {
		return ports[(index++) % ports.size()];
	}

};

class TextSecureServer {
	//
};

} // namespace sinl

#endif // _API_H_
