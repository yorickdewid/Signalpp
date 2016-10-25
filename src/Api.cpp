#include "Api.h"
#include "Base64.h"

#include <sstream>

#include <curl/curl.h>

#define USERAGENT 	"Signal++/0.1"

using namespace signalpp;

struct dataObject {
	const char *readptr;
	long sizeleft;
};

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
	struct dataObject *writeback = (struct dataObject *)userdata;

	if (size * nmemb < 1)
		return 0;

	if (writeback->sizeleft) {
		// *(char *)ptr = writeback->readptr[0]; /* copy one single byte */ 
		*(char *)ptr = writeback->readptr[0]; /* copy one single byte */
		printf("%c", *(char *)ptr);
		writeback->readptr++;                 /* advance pointer */ 
		writeback->sizeleft--;                /* less data left */ 
		return 1;                        /* we return 1 byte at a time! */ 
	}

	return 0;
}

//TODO: verification callback
bool TextSecureServer::performCall(enum urlCall call, enum httpType type, const std::string& param, const std::string& data) {
	CURL *curl;
	CURLcode res;
	bool response = true;

	std::ostringstream url;
	url << m_url;
	url << ':';
	url << m_portManager.getPort();
	url << '/';
	url << endpoint[call];
	url << param;

	SIGNAL_LOG_DEBUG << "Perform API call with:";
	SIGNAL_LOG_DEBUG << "  url: " << url.str();
	SIGNAL_LOG_DEBUG << "  type: " << type;
	SIGNAL_LOG_DEBUG << "  data: " << data;
	SIGNAL_LOG_DEBUG << "  Content-Type: application/json";
	SIGNAL_LOG_DEBUG << "  user: " << m_username;
	SIGNAL_LOG_DEBUG << "  password: " << m_password;

	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());

		/* Set HTTP type */
		switch (type) {
			case GET:
				curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
				break;
			case PUT:
				curl_easy_setopt(curl, CURLOPT_PUT, 1L);
				break;
			case POST:
				curl_easy_setopt(curl, CURLOPT_POST, 1L);
				break;
			case DELETE:
				curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE"); 
				break;
		}

		//TODO: data
		if (!data.empty()) {
			struct dataObject writeback;

			writeback.readptr = data.c_str();
			writeback.sizeleft = (long)data.size();

			curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

			/* pointer to pass to the read function */ 
			curl_easy_setopt(curl, CURLOPT_READDATA, &writeback);
		}

#ifdef SKIP_PEER_VERIFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
		struct curl_slist *headers = nullptr;

		headers = curl_slist_append(headers, "Content-Type: application/json");

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
			SIGNAL_LOG_ERROR << "curl_easy_perform() failed: " << curl_easy_strerror(res);
			response = false;
		}

		long code = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);

		std::string message;
		switch (code) {
			case 200:
				message = "Alreaiighhtt....";
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

		SIGNAL_LOG_DEBUG << "Response code: " << code;
		SIGNAL_LOG_ERROR << message;

		curl_slist_free_all(headers); /* free the header list */

		/* always cleanup */ 
		curl_easy_cleanup(curl);
	} else {
		response = false;
	}

	curl_global_cleanup();

	return response;
}

bool TextSecureServer::confirmCode(const std::string& number,
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
	return performCall(call, PUT, urlPrefix + code, jsonData);
}

Websocket *TextSecureServer::getMessageSocket() {
	std::string url = getUrl();

	replace(url, "https://", "wss://");
	replace(url, "http://", "ws://");

	url += "/v1/websocket/?login=" + Base64::Encode(m_username) + "&password=" + Base64::Encode(m_password) + "&agent=OWD";

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
