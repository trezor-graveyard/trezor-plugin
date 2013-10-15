#ifndef TREZOR_EXCEPTIONS_H
#define TREZOR_EXCEPTIONS_H

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
        : FB::script_error("Failed to load configuration: " + msg) {}
};

struct OpenError : FB::script_error
{
    OpenError() : FB::script_error("Failed to open device") {}
};

struct ReadTimeout : FB::script_error {
    ReadTimeout() : FB::script_error("Read timeout") {}
};

struct MessageTypeUnknown : FB::script_error {
    MessageTypeUnknown() : FB::script_error("Message type unknown") {}
};

#endif // TREZOR_EXCEPTIONS_H
