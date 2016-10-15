#ifndef _API_H_
#define _API_H_

#include "Logger.h"
#include "Helper.h"

#include <vector>
#include <string>
#include <sstream>
#include <map>

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

	enum urlCall {
		ACCOUNTS,
		DEVICES,
		KEYS,
		MESSAGES,
		ATTACHMENT,
	};

	enum httpType {
		GET,
		PUT,
		POST,
		DELETE,
	};

	std::map<enum urlCall, std::string> endpoint;
	
	void performCall(enum urlCall call, enum httpType type, const std::string& param);

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
			m_portManager.add(ports[i]);
			++i;
		}

		/* API call endpoints */
		endpoint[ACCOUNTS]   = "v1/accounts";
		endpoint[DEVICES]    = "v1/devices";
		endpoint[KEYS]       = "v2/keys";
		endpoint[MESSAGES]   = "v1/messages";
		endpoint[ATTACHMENT] = "v1/attachments";
	}

	inline std::string getUrl() {
		std::ostringstream os;
		os << m_url;
		os << ':';
		os << m_portManager.getPort();
		return os.str();
	}

	inline void requestVerificationSMS(const std::string& number) {
		return performCall(ACCOUNTS, GET, "/sms/code/" + number);
	}

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

	int getProvisioningSocket() {
		std::string url = getUrl();

		replace(url, "https://", "wss://");
		replace(url, "http://", "ws://");

		url += "/v1/websocket/provisioning/?agent=OWD";

		SIGNAL_LOG_DEBUG << "Opening websocket to " << url;
		// return new WebSocket(url);
		return 1;
	}
};

} // namespace signal

#endif // _API_H_
