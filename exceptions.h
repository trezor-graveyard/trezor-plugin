#ifndef EXCEPTIONS_H
#define	EXCEPTIONS_H

#include "JSExceptions.h"

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

struct WriteError : FB::script_error {
    WriteError()
        : FB::script_error("Write error") {}
    ~WriteError() throw() {}
};

struct MessageTypeError : FB::script_error {
    MessageTypeError()
        : FB::script_error("Message type unknown") {}
    ~MessageTypeError() throw() {}
};

#endif	/* EXCEPTIONS_H */
