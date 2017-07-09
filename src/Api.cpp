#include "Api.h"
#include "Base64.h"
#include "KeyHelper.h"

#include <sstream>

#include <curl/curl.h>

#define USERAGENT 	"Signal++/0.1"

using namespace signalpp;

using json = nlohmann::json;

struct dataObject {
	const char *readptr;
	long sizeleft;
};

struct dataObjectWrite {
	char *writeptr;
	long size;
};

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
	struct dataObject *writeback = (struct dataObject *)userdata;

	if (size * nmemb < 1)
		return 0;

	if (writeback->sizeleft) {
		*(char *)ptr = writeback->readptr[0];
		printf("%c", *(char *)ptr);
		writeback->readptr++;
		writeback->sizeleft--;
		return 1;
	}

	return 0;
}

static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
	size_t realsize = size * nmemb;
	struct dataObjectWrite *mem = (struct dataObjectWrite *)userdata;

	mem->writeptr = (char *)realloc(mem->writeptr, mem->size + realsize + 1);
	if (!mem->writeptr) {
		SIGNAL_LOG_DEBUG << "Memory allocation failed";
		return 0;
	}

	memcpy(&(mem->writeptr[mem->size]), ptr, realsize);
	mem->size += realsize;
	mem->writeptr[mem->size] = '\0';

	return realsize;
}

//TODO: verification callback
std::tuple<std::string, int> TextSecureServer::performCall(enum urlCall call, enum httpType type, const std::string& param, const std::string& data) {
	CURL *curl;
	CURLcode res;
	struct dataObjectWrite writeback;
	bool response = true;

	writeback.writeptr = (char *)malloc(1);
	writeback.size = 0;

	std::ostringstream url;
	url << m_url;
	url << ':';
	url << m_portManager.getPort();
	url << '/';
	url << endpoint[call];
	url << param;

	std::cout << "Perform API call with:" << std::endl;
	std::cout << "  url: " << url.str() << std::endl;
	std::cout << "  type: " << type << std::endl;
	std::cout << "  data: " << data << std::endl;
	std::cout << "  Content-Type: application/json" << std::endl;
	std::cout << "  user: " << m_username << std::endl;
	std::cout << "  password: " << m_password << std::endl;

	curl_global_init(CURL_GLOBAL_DEFAULT);

	long code = 0;
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());

		/* Set HTTP type */
		switch (type) {
			case _GET:
				curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
				break;
			case _PUT:
				curl_easy_setopt(curl, CURLOPT_PUT, 1L);
				break;
			case _POST:
				curl_easy_setopt(curl, CURLOPT_POST, 1L);
				break;
			case _DELETE:
				curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE"); 
				break;
		}

		//TODO: data
		if (!data.empty()) {
			struct dataObject readback;

			readback.readptr = data.c_str();
			readback.sizeleft = (long)data.size();

			curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

			/* Pointer to pass to the read function */ 
			curl_easy_setopt(curl, CURLOPT_READDATA, &readback);
		}

		/* Send all data to this function */ 
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

		/* Pass struct to the callback function */ 
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeback);

#ifdef SKIP_PEER_VERIFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
		struct curl_slist *headers = nullptr;

		headers = curl_slist_append(headers, "Content-Type: application/json");
		headers = curl_slist_append(headers, "X-Signal-Agent: SNPP");

		/* Pass custom headers */
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		/* User agent */
		curl_easy_setopt(curl, CURLOPT_USERAGENT, USERAGENT);

		/* Basic authentication */
		if (!m_username.empty() && !m_password.empty()) {
			curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_BASIC);
			curl_easy_setopt(curl, CURLOPT_USERPWD, (m_username + ":" + m_password).c_str());
		}

		/* Perform the request */ 
		// res = CURLE_OK;
		res = curl_easy_perform(curl);

		/* Check for errors */ 
		if (res != CURLE_OK) {
			//SIGNAL_LOG_ERROR << "curl_easy_perform() failed: " << curl_easy_strerror(res);
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
			response = false;
		}

		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);

		std::string message;
		switch (code) {
			case 200:
			case 204:
				message = "Success.";
				break;
			case -1:
				message = "Failed to connect to the server, please check your network connection.";
				break;
			case 413:
				message = "Rate limit exceeded, please try again later.";
				break;
			case 403:
				message = "Invalid code, please try again.";
				break;
			case 417:
				//TODO: This shouldn't be a thing?, but its in the API doc?
				message = "Number already registered.";
				break;
			case 418:
				message = "Bring that teapot.";
				break;
			case 401:
				message = "Invalid authentication, most likely someone re-registered and invalidated our registration.";
				break;
			case 404:
				message = "Number is not registered.";
				break;
			case 500:
				message = "Internal server error.";
				break;
			default:
				message = "The server rejected our query, please file a bug report.";
		}

		if (writeback.size)
			std::cout << writeback.writeptr << std::endl;
		std::cout << "Response code: " << code << std::endl;
		std::cout << message << std::endl;

		curl_slist_free_all(headers); /* free the header list */

		/* always cleanup */ 
		curl_easy_cleanup(curl);
	} else {
		response = false;
	}

	curl_global_cleanup();

	if (!writeback.size || !response) {
		return std::make_tuple("", code);//TODO: throw
	}

	return std::make_tuple(std::string(writeback.writeptr, writeback.size), code);
}

int TextSecureServer::confirmCode(const std::string& number,
									const std::string& code,
									const std::string& password,
									std::string& signaling_key,
									short registrationId,
									const std::string& deviceName) {
	std::string jsonData = "{"
		"\"signalingKey\":\"" + Base64::Encode(signaling_key) + "\","
		"\"supportsSms\":false,"
		"\"fetchesMessages\":true,"
		"\"registrationId\":" + std::to_string(registrationId);

	enum urlCall call;
	std::string urlPrefix;

	if (!deviceName.empty()) {
		jsonData += ",\"name\":\"" + deviceName + "\"}";
		call = DEVICES;
		urlPrefix = "/";
	} else {
		jsonData += "}";
		call = ACCOUNTS;
		urlPrefix = "/code/";
	}

	m_username = number;
	m_password = password;
	std::string deviceId = std::get<0>(performCall(call, _PUT, urlPrefix + code, jsonData));
	if (deviceId.empty()) {
		return 0;
	}

	char idchar = deviceId[deviceId.find(":") + 1];
	return (idchar - '0') % 48;  
}

void TextSecureServer::registerKeys(prekey::result& result) {
	std::string keys = "{"
		"\"identityKey\":\"" + KeyHelper::encodePublicKey(result.identityKey) + "\",";

	std::string signature = Base64::EncodeRaw(
		signal_buffer_data(result.signedPreKey.signature),
		signal_buffer_len(result.signedPreKey.signature));

	keys += "\"signedPreKey\":{";
	keys += "\"keyId\":" + std::to_string(result.signedPreKey.keyId) + ",";
	keys += "\"publicKey\":\"" + KeyHelper::encodePublicKey(result.signedPreKey.publicKey) + "\",";
	keys += "\"signature\":\"" + signature + "\"";
	keys += "},";

	keys += "\"preKeys\":[";

	bool useComma = false;
	for (const auto& preKeyPair : result.preKeys) {
		if (useComma) {
			keys += ",{";
		} else {
			keys += "{";
			useComma = true;
		}
		keys += "\"keyId\":" + std::to_string(preKeyPair.keyId) + ",";
		keys += "\"publicKey\":\"" + KeyHelper::encodePublicKey(preKeyPair.publicKey) + "\"";
		keys += "}";
	}

	keys += "],";

	keys += "\"lastResortKey\":{\"keyId\":2147483647,\"publicKey\":\"NDI=\"}";
	keys += "}";

	performCall(KEYS, _PUT, "", keys);
}

json TextSecureServer::getKeysForNumber(std::string& number, int deviceId) {
	std::string uri = "/" + number + "/" + std::to_string(deviceId);
	if (!deviceId)
		std::string uri = "/" + number + "/*";

	auto response = performCall(KEYS, _GET, uri);
	if (std::get<1>(response) != 200) {
		return nullptr;
	}

	auto res = json::parse(std::get<0>(response));
	// auto res = R"({"identityKey":"BTFwfbnoMaimVLHznxWKDTruswF/FKAOKi3qDVai3UJr","devices":[{"deviceId":1,"registrationId":6922,"signedPreKey":{"keyId":8731799,"publicKey":"BUnRGYXIT+MQ1P5NaoDWgG4FtxEbB5/MEMJ6ME5qLKUJ","signature":"wogO8wlAbDvrSW6SpZwjdig8edd6gfNsEYSeAO+4rOlUsqApCmwyA/30mp1AL8wVcDpRX53tbNaTuA+N2VjbgA"},"preKey":{"keyId":12731962,"publicKey":"BQUzo4DmbT7uYDNhC+MY3KSTbqLT9pP+zsSxwVmrJdkT"}}]})"_json;
	if (!res["devices"].is_array()) {
		//SIGNAL_LOG_ERROR << "Invalid response";
		std::cerr << "Invalid response" << std::endl;
	}

	res["identityKey"] = Base64::Decode(res["identityKey"].get<std::string>());

	for (auto& device : res["devices"]) {
		device["signedPreKey"]["publicKey"] = Base64::Decode(device["signedPreKey"]["publicKey"]);
		device["signedPreKey"]["signature"] = Base64::Decode(device["signedPreKey"]["signature"]);
		device["preKey"]["publicKey"] = Base64::Decode(device["preKey"]["publicKey"]);
	}

	return res;
}

Websocket *TextSecureServer::getMessageSocket() {
	std::string url = getUrl();

	replace(url, "https://", "wss://");
	replace(url, "http://", "ws://");

	url += "/v1/websocket/?login=" + Url::Encode(m_username) + "&password=" + Url::Encode(m_password) + "&agent=OWD";

	SIGNAL_LOG_DEBUG << "Websocket to " << url;

	return new Websocket(url);
}

Websocket *TextSecureServer::getProvisioningSocket() {
	std::string url = getUrl();

	replace(url, "https://", "wss://");
	replace(url, "http://", "ws://");

	url += "/v1/websocket/provisioning/?agent=OWD";

	SIGNAL_LOG_DEBUG << "Websocket to " << url;

	return new Websocket(url);
}
