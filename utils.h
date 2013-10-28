#ifndef TREZOR_UTILS_H
#define TREZOR_UTILS_H

#include <string>
#include <sstream>

#ifdef _MSC_VER
#  include <cstdint>
#else
#  include <stdint.h>
#endif

namespace utils {

//
// Crypto
//

static const int SIGNATURE_LENGTH = 64; // in bytes

bool signature_verify(const uint8_t *sig,
                      const uint8_t *data,
                      size_t datalen);

//
// UTF-8 codec
//

std::string utf8_encode(const std::wstring &str);
std::wstring utf8_decode(const std::string &str);

//
// Hex codec
//

template <typename T>
inline std::string hex_encode(const T &val)
{
    std::ostringstream stream;
    stream << std::hex << val;
    return stream.str();
}

template <>
std::string hex_encode<std::string>(const std::string &str);
std::string hex_decode(const std::string &hex);

}

#endif // TREZOR_UTILS_H
