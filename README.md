# weft

[![ci](https://github.com/lvntky/weft/actions/workflows/ci.yml/badge.svg)](https://github.com/lvntky/weft/actions/workflows/ci.yml)
[![license](https://img.shields.io/github/license/lvntky/weft)](LICENSE)
![C11](https://img.shields.io/badge/C-11-blue)
![platform](https://img.shields.io/badge/platform-Linux%205.19%2B-lightgrey)

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

## What it is ?

Weft is a library primarely supports 3 operations:

- **Stackfull coroutines.** OS threads are expensive. Costs megabytes of stack and syscall overhead per switch. A *weft task* costs 64KB and a few nano seconds. 
- **A Work Stealing Scheduler.** Tasks are distributed across N worker thread on io_uring instance each. Idle workes steal from busy ones.
- **Task Groups.** A group does not close until it's children finish. Cancel a
  group and cancellation propagates down the tree, pending kernel operations
  included.

## Why

C programmers building high-concurrency network services — proxies, databases,
brokers, gateways — currently choose between libuv and writing their own event
loop. libuv was designed in 2011 around callbacks and epoll; io_uring support
came later and partially. liburing is a syscall wrapper, not a runtime: you
still build everything above it yourself.

Rust has Tokio. Go has goroutines. Zig has libxev. C has a gap. There is actually libraries like libdill which is unmainted pre io-uring and libmill which is also unmaintained library. Weft is more io-uring native and modern apporach to async.

## Status

Pre-alpha. API will change.

## Requirements

Linux 5.19+, liburing 2.3+, C11 compiler.

## Build

## Acknowledgements

- [x86-64 psABI](https://gitlab.com/x86-psABIs/x86-64-ABI)
