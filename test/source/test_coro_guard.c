#include <stdio.h>
#include <string.h>

#include "internal.h"

static void *main_sp;

static volatile char sink;

static void overflow(void *arg)
{
    (void)arg;
    for (long depth = 0;; depth++) {
        volatile char frame[4096];
        memset((void *)frame, (int)depth, sizeof frame);
        sink = frame[0];
        if (depth > (1L << 20)) {
            fprintf(stderr, "guard page never hit\n");
            return;
        }
    }
}

int main(void)
{
    weft__coro *c = weft__coro_new(overflow, NULL, 64 * 1024);
    if (c == NULL) {
        return 1;
    }
    c->ret = &main_sp;
    weft__coro_switch(&main_sp, c->sp);
    fprintf(stderr, "returned without crashing\n");
    return 0;
}
