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
	std::string m_url;
	std::string m_username;
	std::string m_password;
	std::string m_attachUrl;
	PortManager m_portManager;

  public:
    TextSecureServer(const std::string& url,
    					const std::string& username,
    					const std::string& password,
    					const std::string& attachment_server_url,
    					const unsigned short ports[])
    : m_url(url)
    , m_username(username)
    , m_password(password)
    , m_attachUrl(attachment_server_url) {

    	int i = 0;
    	while (ports[i] > 0) {
    		m_portManager.add(i);
    		++i;
    	}

    }
};

} // namespace sinl

#endif // _API_H_
