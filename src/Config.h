#ifndef _CONFIG_H_
#define _CONFIG_H_

namespace signal {

/* Version */
constexpr int majorVersion = 0;
constexpr int minorVersion = 1;
constexpr int patchVersion = 0;

/* Connection strings */
static const char serverUrl[] = "https://textsecure-service-ca.whispersystems.org";
// static const char websocketUrl[] = "wss://textsecure-service-ca.whispersystems.org";
static const char AttachmentServerUrl[] = "https://whispersystems-textsecure-attachments.s3.amazonaws.com";
static const unsigned short serverPorts[] = {80, 4433, 8443, 0};

}; // namespace signal

#endif // _CONFIG_H_
