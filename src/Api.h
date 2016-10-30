#ifndef _API_H_
#define _API_H_

#include "Logger.h"
#include "Helper.h"
#include "Websocket.h"
#include "PreKeyResult.h"
#include "Json.h"

#include <vector>
#include <string>
#include <sstream>
#include <map>

namespace signalpp {

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
	
	std::string performCall(enum urlCall call, enum httpType type, const std::string& param = "", const std::string& data = "");

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

	inline void setUsername(const std::string& username) {
		m_username = username;
	}

	inline std::string getUrl() {
		std::ostringstream os;
		os << m_url;
		os << ':';
		os << m_portManager.getPort();
		return os.str();
	}

	inline bool requestVerificationSMS(const std::string& number) {
		return !performCall(ACCOUNTS, GET, "/sms/code/" + number).empty();
	}

	inline bool requestVerificationVoice(const std::string& number) {
		return !performCall(ACCOUNTS, GET, "/voice/code/" + number).empty();
	}

	int confirmCode(const std::string& number,
						const std::string& code,
						const std::string& password,
						std::string& signaling_key,
						short registrationId,
						const std::string& deviceName);

	bool getDevices() {
		return !performCall(DEVICES, GET).empty();
	}

	void registerKeys(prekey::result& result);

	void getMyKeys() {}
	nlohmann::json getKeysForNumber(std::string& number, int deviceId = 0);

	bool sendMessages(const std::string& destination, char *messageArray, int timestamp) {
		// var jsonData = { messages: messageArray, timestamp: timestamp};

		//TODO: pass jsonData as data
		return !performCall(MESSAGES, PUT, "/" + destination).empty();
	}

	void getAttachment() {}
	void putAttachment() {}

	Websocket *getMessageSocket();
	Websocket *getProvisioningSocket();
};

} // namespace signalpp

#endif // _API_H_
