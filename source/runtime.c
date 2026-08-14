#include <stdarg.h>
#include <stdio.h>

#include "internal.h"

_Thread_local weft__worker *weft__self = NULL;

const char *weft_version_string(void)
{
    return WEFT_VERSION_STRING;
}

int weft_version_num(void)
{
    return WEFT_VERSION_NUM;
}

const char *weft_strerror(int err)
{
    switch (err) {
    case WEFT_OK:
        return "success";
    case WEFT_ECANCELED:
        return "operation canceled";
    case WEFT_ETIMEDOUT:
        return "deadline exceeded";
    case WEFT_ENOMEM:
        return "out of memory";
    case WEFT_EUNSUP:
        return "unsupported by kernel";
    case WEFT_ESTATE:
        return "invalid runtime state";
    default:
        break;
    }
    if (err < 0) {
        return strerror(-err);
    }
    return "unknown error";
}

void weft__trace(const char *file, int line, const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "weft: %s:%d: ", file, line);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
}

void weft__panic(const char *file, int line, const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "weft: panic at %s:%d: ", file, line);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    abort();
}
