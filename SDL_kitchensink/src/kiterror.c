#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "kitchensink/kitchensink.h"

#define KIT_ERRBUFSIZE 1024

static char _error_available = false;
static char _error_message[KIT_ERRBUFSIZE] = "\0";

const char* Kit_GetError() {
    if(_error_available) {
        _error_available = false;
        return _error_message;
    }
    return NULL;
}

void Kit_SetError(const char* fmt, ...) {
    assert(fmt != NULL);
    va_list args;
    va_start(args, fmt);
    vsnprintf(_error_message, KIT_ERRBUFSIZE, (char*)fmt, args);
    va_end(args);
    _error_available = true;
}

void Kit_ClearError() {
    _error_message[0] = 0;
    _error_available = false;
}
