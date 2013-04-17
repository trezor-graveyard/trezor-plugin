#ifndef EXCEPTIONS_H
#define	EXCEPTIONS_H

#include "JSExceptions.h"

struct ActionCanceled : FB::script_error {
    ActionCanceled()
        : FB::script_error("Action cancelled by user")
    { }
    ~ActionCanceled() throw() { }
};

struct ReadTimeout : FB::script_error {
    ReadTimeout()
        : FB::script_error("Read timeout")
    { }
    ~ReadTimeout() throw() { }
};

#endif	/* EXCEPTIONS_H */
