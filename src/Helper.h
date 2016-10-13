#ifndef _HELPER_H_
#define _HELPER_H_

namespace signal {

namespace {

// getDeviceName()

static const char *getVersion() {
	static char buf[8];
	sprintf(buf, "%u.%u.%u", majorVersion, minorVersion, patchVersion);
	return buf;
}

} // namespace

} // namespace signal

#endif // _HELPER_H_

