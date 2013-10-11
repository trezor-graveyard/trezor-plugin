#ifndef TREZOR_UTILS_H
#define TREZOR_UTILS_H

#include <string>
#include <sstream>
#include <boost/algorithm/hex.hpp>

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
inline std::string hex_encode<std::string>(const std::string &str)
{
    std::ostringstream stream;
    std::ostream_iterator<char> iterator(stream);
    boost::algorithm::hex(str, iterator);
    return stream.str();
}

inline std::string hex_decode(const std::string &hex)
{
    std::ostringstream stream;
    std::ostream_iterator<char> iterator(stream);
    boost::algorithm::unhex(hex, iterator);
    return stream.str();
}

#endif // TREZOR_UTILS_H
