#ifndef EXCEPTIONS_H
#define	EXCEPTIONS_H

#include "JSExceptions.h"

struct ActionCanceled : FB::script_error
{
    ActionCanceled()
        : FB::script_error("Action cancelled by user") {}
    ~ActionCanceled() throw() {}
};

struct PinInvalid : FB::script_error
{
    PinInvalid()
        : FB::script_error("Invalid PIN") {}
    ~PinInvalid() throw() {}
};

struct ReadTimeout : FB::script_error {
    ReadTimeout()
        : FB::script_error("Read timeout") {}
    ~ReadTimeout() throw() {}
};

struct ReadError : FB::script_error {
    ReadError()
        : FB::script_error("Read error") {}
    ~ReadError() throw() {}
};

struct MessageTypeError : FB::script_error {
    MessageTypeError()
        : FB::script_error("Message type unknown") {}
    ~MessageTypeError() throw() {}
};

#endif	/* EXCEPTIONS_H */
