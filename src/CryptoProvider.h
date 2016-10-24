#ifndef _CRYPTOPROVIDER_H_
#define _CRYPTOPROVIDER_H_

#include <signal_protocol.h>

#include <string>

namespace signalpp {

namespace CryptoProvider {

void hook(signal_context *context);

std::string HMAC(std::string& key, std::string& data);

bool verifyMAC(std::string& data, std::string& key, std::string& mac, size_t length);

std::string decrypt(const std::string& key, const std::string& ciphertext, const std::string& iv);

} // namespace CryptoProvider

} // namespace signalpp

#endif // _CRYPTOPROVIDER_H_
