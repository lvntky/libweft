#ifndef WEFT_INTERNAL_H
#define WEFT_INTERNAL_H

#include <assert.h>
#include <errno.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <liburing.h>

#include "weft/weft.h"

#include "config.h"

#define WEFT_LIKELY(x)    __builtin_expect(!!(x), 1)
#define WEFT_UNLIKELY(x)  __builtin_expect(!!(x), 0)
#define WEFT_ALIGNED(n)   __attribute__((aligned(n)))
#define WEFT_CACHE_ALIGN  WEFT_ALIGNED(WEFT_CACHELINE_SIZE)
#define WEFT_UNUSED(x)    ((void)(x))
#define WEFT_ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

#define weft_container_of(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))

#define WEFT_MIN(a, b) ((a) < (b) ? (a) : (b))
#define WEFT_MAX(a, b) ((a) > (b) ? (a) : (b))

#ifndef NDEBUG
#  define WEFT_ASSERT(x)  assert(x)
#  define WEFT_TRACE(...) weft__trace(__FILE__, __LINE__, __VA_ARGS__)
#else
#  define WEFT_ASSERT(x)  ((void)0)
#  define WEFT_TRACE(...) ((void)0)
#endif

#define WEFT_PANIC(...) weft__panic(__FILE__, __LINE__, __VA_ARGS__)

typedef struct weft__coro weft__coro;
typedef struct weft__worker weft__worker;
typedef struct weft__rt weft__rt;
typedef struct weft__deque weft__deque;

enum weft__coro_state {
    WEFT__CORO_NEW,
    WEFT__CORO_READY,
    WEFT__CORO_RUNNING,
    WEFT__CORO_BLOCKED,
    WEFT__CORO_DONE
};

struct weft__coro {
    void *sp;
    void *stack;
    size_t stack_size;
    weft_fn fn;
    void *arg;
    weft_group *group;
    weft__coro *next;
    int result;
    _Atomic int state;
};

struct weft_group {
    _Atomic unsigned pending;
    _Atomic bool canceled;
    weft__coro *waiter;
    weft_group *parent;
};

struct weft__worker {
    weft__rt *rt;
    struct io_uring ring;
    weft__deque *runq;
    weft__coro *current;
    void *sched_sp;
    unsigned id;
    int wake_fd;
} WEFT_CACHE_ALIGN;

struct weft__rt {
    weft__worker *workers;
    unsigned nworkers;
    weft_config cfg;
    _Atomic int stopping;
    _Atomic int live;
};

WEFT_LOCAL extern _Thread_local weft__worker *weft__self;

WEFT_LOCAL void weft__trace(const char *file, int line, const char *fmt, ...) WEFT_PRINTF(3, 4);
WEFT_LOCAL WEFT_NORETURN void weft__panic(const char *file, int line, const char *fmt, ...)
    WEFT_PRINTF(3, 4);

WEFT_LOCAL weft__coro *weft__coro_new(weft_fn fn, void *arg, size_t stack_size);
WEFT_LOCAL void weft__coro_free(weft__coro *c);
WEFT_LOCAL void weft__coro_switch(void **save_sp, void *load_sp);
WEFT_LOCAL void weft__coro_trampoline(void);

WEFT_LOCAL void weft__sched_push(weft__worker *w, weft__coro *c);
WEFT_LOCAL weft__coro *weft__sched_next(weft__worker *w);
WEFT_LOCAL void weft__sched_yield(int new_state);
WEFT_LOCAL void weft__sched_loop(weft__worker *w);

WEFT_LOCAL int weft__ring_init(weft__worker *w, unsigned entries, unsigned flags);
WEFT_LOCAL void weft__ring_destroy(weft__worker *w);
WEFT_LOCAL struct io_uring_sqe *weft__sqe_get(weft__worker *w);
WEFT_LOCAL int weft__await(weft__worker *w, struct io_uring_sqe *sqe, int64_t deadline_ms);
WEFT_LOCAL void weft__ring_reap(weft__worker *w, int block);

WEFT_LOCAL weft__deque *weft__deque_new(size_t cap);
WEFT_LOCAL void weft__deque_free(weft__deque *d);
WEFT_LOCAL bool weft__deque_push(weft__deque *d, weft__coro *c);
WEFT_LOCAL weft__coro *weft__deque_pop(weft__deque *d);
WEFT_LOCAL weft__coro *weft__deque_steal(weft__deque *d);

#endif
