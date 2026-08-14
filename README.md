# weft

Structured concurrency for C, built on io_uring.

Write blocking-looking code. Get non-blocking execution, mandatory
deadlines, and no leaked tasks.

```c
#include <weft/weft.h>

static void conn(void *arg) {
    int fd = (int)(intptr_t)arg;
    char buf[4096];
    for (;;) {
        int n = weft_read(fd, buf, sizeof buf, WEFT_SEC(30));
        if (n <= 0) break;
        if (weft_write_all(fd, buf, n, WEFT_SEC(30)) < 0) break;
    }
    weft_close(fd);
}
```

## Why

- **Deadlines are not optional.** Every I/O call takes one. Hung
  connections are a bug you cannot write.
- **Task groups, not orphans.** A group does not close until its
  children finish or are canceled.
- **No callbacks.** Stackful coroutines over a work-stealing scheduler.

## Status

Pre-alpha. API will change.

## Requirements

Linux 5.19+, liburing 2.3+, C11 compiler.

## Build
