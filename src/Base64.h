#ifndef _BASE64_H_
#define _BASE64_H_

#include <string>

namespace signalpp {

namespace Base64 {

std::string EncodeRaw(unsigned char const *bytes_to_encode, unsigned int in_len);

inline std::string Encode(std::string& plain_string) {
	return EncodeRaw((unsigned char const *)plain_string.c_str(), (unsigned int)plain_string.size());
}

std::string Decode(std::string const& encoded_string);

} // namespace Base64

} // namespace signalpp

#endif // _BASE64_H_

