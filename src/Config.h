#ifndef _CONFIG_H_
#define _CONFIG_H_

namespace signalpp {

/* Version */
constexpr int majorVersion = 0;
constexpr int minorVersion = 1;
constexpr int patchVersion = 0;

/* Connection strings */
static const char serverUrl[] = "http://127.0.0.1";//"https://textsecure-service.whispersystems.org";
static const char attachmentServerUrl[] = "https://whispersystems-textsecure-attachments.s3.amazonaws.com";
static const unsigned short serverPorts[] = {110, 0}; // 443, 80, 4433, 8443,

}; // namespace signalpp

#endif // _CONFIG_H_
