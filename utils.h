#ifndef UTILS_H
#define	UTILS_H

#include <string>
#include <sstream>
#include <boost/algorithm/hex.hpp>

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

#endif
