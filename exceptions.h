#pragma once

#include <boost/lexical_cast.hpp>

#include "JSExceptions.h"

struct ReadError : FB::script_error
{
    ReadError(const std::string &msg)
        : FB::script_error("Read error: " + msg) {}
};

struct WriteError : FB::script_error
{
    WriteError(const std::string &msg)
        : FB::script_error("Write error: " + msg) {}
};

struct ConfigurationError : FB::script_error
{
    ConfigurationError(const std::string &msg)
        : FB::script_error("Configuration error: " + msg) {}
};

struct MessageTypeUnknown : FB::script_error
{
    MessageTypeUnknown(uint16_t type)
        : FB::script_error("Message type unknown: "
                           + boost::lexical_cast<std::string>(type)) {}
    MessageTypeUnknown(const std::string &type)
        : FB::script_error("Message type unknown: '" + type + "'") {}
};

struct OpenError : FB::script_error
{
    OpenError() : FB::script_error("Opening device failed") {}
};

struct ReadTimeout : FB::script_error
{
    ReadTimeout() : FB::script_error("Reading timed out") {}
};