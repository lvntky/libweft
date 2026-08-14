#include <stdint.h>
#include <stdio.h>

#include <weft/weft.h>

static void conn(void *arg)
{
    int fd = (int)(intptr_t)arg;
    char buf[4096];

    for (;;) {
        int n = weft_read(fd, buf, sizeof buf, WEFT_SEC(30));
        if (n <= 0) {
            break;
        }
        if (weft_write_all(fd, buf, (size_t)n, WEFT_SEC(30)) < 0) {
            break;
        }
    }
    weft_close(fd);
}

static void server(void *arg)
{
    uint16_t port = (uint16_t)(uintptr_t)arg;

    int lfd = weft_listen("0.0.0.0", port);
    if (lfd < 0) {
        fprintf(stderr, "listen: %s\n", weft_strerror(lfd));
        return;
    }
    fprintf(stderr, "echo listening on :%u\n", port);

    weft_group *g = weft_group_begin();
    for (;;) {
        int fd = weft_accept(lfd, WEFT_NO_DEADLINE);
        if (fd < 0) {
            break;
        }
        weft_go(g, conn, (void *)(intptr_t)fd);
    }
    weft_group_end(g);
    weft_close(lfd);
}

int main(void)
{
    weft_config cfg = WEFT_CONFIG_INIT;
    int rc = weft_run(server, (void *)(uintptr_t)8080, &cfg);
    if (rc != WEFT_OK) {
        fprintf(stderr, "weft_run: %s\n", weft_strerror(rc));
        return 1;
    }
    return 0;
}
