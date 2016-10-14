#ifndef _HELPER_H_
#define _HELPER_H_

#include "Config.h"

#include <unistd.h>

namespace signal {

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

} // namespace

} // namespace signal

#endif // _HELPER_H_

