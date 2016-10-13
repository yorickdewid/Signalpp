#ifndef _API_H_
#define _API_H_

#include <vector>

namespace signal {

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

	/*enum urlClass {
        accounts   : "v1/accounts",
        devices    : "v1/devices",
        keys       : "v2/keys",
        messages   : "v1/messages",
        attachment : "v1/attachments"
	};*/

  public:
	TextSecureServer(const std::string& url,
						const std::string& username,
						const std::string& password,
						const unsigned short ports[],
						const std::string& attachment_server_url = "")
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

	void getUrl() {}
	void performCall() {}
	void requestVerificationSMS(const std::string& number) {}
	void requestVerificationVoice(const std::string& number) {}
	void confirmCode() {}
	void getDevices() {}
	void registerKeys() {}
	void getMyKeys() {}
	void getKeysForNumber() {}
	void sendMessages() {}
	void getAttachment() {}
	void putAttachment() {}
	void getMessageSocket() {}
	void getProvisioningSocket() {}
};

} // namespace signal

#endif // _API_H_
