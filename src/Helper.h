#ifndef _HELPER_H_
#define _HELPER_H_

#include "Config.h"

#include <iomanip>
#include <ctime>
//#include <unistd.h>

namespace signalpp {

namespace {

static const char *getDeviceName() {
	static char hostname[1024];
	gethostname(hostname, 1024);
	return hostname;
}

static const char *getVersion() {
	static char version[8];
	sprintf(version, "%u.%u.%u", majorVersion, minorVersion, patchVersion);
	return version;
}

static bool replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if(start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

static long int getTimestamp() {
	return static_cast<long int>(time(NULL));
}

static long int getTimestampLong() {
	return (static_cast<long int>(time(NULL)) * 1000) + 123;
}

namespace Url {

static std::string Encode(const std::string &value) {
	std::ostringstream escaped;
	escaped.fill('0');
	escaped << std::hex;

	for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
		std::string::value_type c = (*i);

		/* Keep alphanumeric and other accepted characters intact */
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			escaped << c;
			continue;
		}

		/* Any other characters are percent-encoded */
		escaped << std::uppercase;
		escaped << '%' << std::setw(2) << int((unsigned char) c);
		escaped << std::nouppercase;
	}

	return escaped.str();
}

} // Url

} // namespace

} // namespace signalpp

#endif // _HELPER_H_

