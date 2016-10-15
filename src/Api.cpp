#include "Api.h"

#include <sstream>

#include <curl/curl.h>

using namespace signal;

void TextSecureServer::performCall(enum urlCall call, enum httpType type, const std::string& param) {
	CURL *curl;
	CURLcode res;

	std::ostringstream url;
	url << m_url;
	url << ':';
	url << m_portManager.getPort();
	url << '/';
	url << endpoint[call];
	url << param;

	SIGNAL_LOG_DEBUG << "Perform API call with:";// << m_url << '/' << endpoint[call] << param;
	SIGNAL_LOG_DEBUG << "  url: " << url.str();
	SIGNAL_LOG_DEBUG << "  type: " << type;
	SIGNAL_LOG_DEBUG << "  data: (null)";
	SIGNAL_LOG_DEBUG << "  contentType: application/json; charset=utf-8";
	SIGNAL_LOG_DEBUG << "  user: " << m_username;
	SIGNAL_LOG_DEBUG << "  password: " << m_password;

	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());

#ifdef SKIP_PEER_VERIFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

		/* Perform the request, res will get the return code */ 
		res = CURLE_OK;//curl_easy_perform(curl);

		/* Check for errors */ 
		if (res != CURLE_OK)
			SIGNAL_LOG_ERROR << "curl_easy_perform() failed: " << curl_easy_strerror(res);

		/* always cleanup */ 
		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();

	/*return ajax(null, {
			host        : m_url,
			ports       : this.portManager.ports,
			path        : endpoint[call] + param.urlParameters,
			type        : type,
			data        : param.jsonData && textsecure.utils.jsonThing(param.jsonData),
			contentType : 'application/json; charset=utf-8',
			dataType    : 'json',
			user        : m_username,
			password    : m_password,
			validateResponse: param.validateResponse
	}).catch(function(e) {
		var code = e.code;
		if (code === 200) {
			// happens sometimes when we get no response
			// (TODO: Fix server to return 204? instead)
			return null;
		}
		var message;
		switch (code) {
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
				// TODO: This shouldn't be a thing?, but its in the API doc?
				message = "Number already registered.";
				break;
			case 401:
				message = "Invalid authentication, most likely someone re-registered and invalidated our registration.";
				break;
			case 404:
				message = "Number is not registered.";
				break;
			default:
				message = "The server rejected our query, please file a bug report.";
		}
		e.message = message
		throw e;
	});*/
}


//