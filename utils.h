#ifndef TREZOR_UTILS_H
#define TREZOR_UTILS_H

#include <string>
#include <sstream>

//
// Crypto
//

bool check_signature(const uint8_t *data, size_t datalen,
                     const uint8_t *sig, size_t siglen);

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

#endif // TREZOR_UTILS_H
