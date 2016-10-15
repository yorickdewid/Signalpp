#ifndef _BASE64_H_
#define _BASE64_H_

#include <string>

namespace signalpp {

namespace Base64 {

std::string Encode(unsigned char const *bytes_to_encode, unsigned int in_len);
std::string Decode(std::string const& encoded_string);

} // namespace Base64

} // namespace signalpp

#endif // _BASE64_H_

