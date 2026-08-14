#ifndef WEFT_H
#define WEFT_H

#include <stddef.h>
#include <stdint.h>

#include "weft/attr.h"
#include "weft/version.h"
#include "weft/weft_export.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct weft_group weft_group;

typedef void (*weft_fn)(void *arg);

enum {
    WEFT_OK = 0,
    WEFT_ECANCELED = -1,
    WEFT_ETIMEDOUT = -2,
    WEFT_ENOMEM = -3,
    WEFT_EUNSUP = -4,
    WEFT_ESTATE = -5
};

enum { WEFT_FLAG_NONE = 0u, WEFT_FLAG_SQPOLL = 1u << 0, WEFT_FLAG_SINGLE_CPU = 1u << 1 };

typedef struct {
    size_t size;
    unsigned threads;
    unsigned ring_entries;
    size_t stack_size;
    unsigned flags;
} weft_config;

#define WEFT_CONFIG_INIT ((weft_config){.size = sizeof(weft_config)})

#define WEFT_NO_DEADLINE ((int64_t)-1)
#define WEFT_MS(x)       ((int64_t)(x))
#define WEFT_SEC(x)      ((int64_t)(x) * 1000)

WEFT_API WEFT_PURE const char *weft_version_string(void);
WEFT_API WEFT_PURE int weft_version_num(void);
WEFT_API WEFT_PURE const char *weft_strerror(int err);

WEFT_API WEFT_NODISCARD WEFT_NONNULL(1) int weft_run(weft_fn entry, void *arg,
                                                     const weft_config *cfg);

WEFT_API WEFT_NODISCARD weft_group *weft_group_begin(void);
WEFT_API WEFT_NONNULL(1) int weft_group_end(weft_group *g);
WEFT_API WEFT_NONNULL(1) void weft_group_cancel(weft_group *g);

WEFT_API WEFT_NONNULL(1, 2) int weft_go(weft_group *g, weft_fn fn, void *arg);

WEFT_API WEFT_NONNULL(1) int weft_listen(const char *host, uint16_t port);
WEFT_API int weft_accept(int fd, int64_t deadline_ms);
WEFT_API WEFT_NONNULL(2) int weft_read(int fd, void *buf, size_t len, int64_t deadline_ms);
WEFT_API WEFT_NONNULL(2) int weft_write_all(int fd, const void *buf, size_t len,
                                            int64_t deadline_ms);
WEFT_API int weft_close(int fd);
WEFT_API int weft_sleep(int64_t ms);
WEFT_API int weft_yield(void);

#ifdef __cplusplus
}
#endif

#endif
