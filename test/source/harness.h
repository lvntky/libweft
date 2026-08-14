#ifndef WEFT_TEST_HARNESS_H
#define WEFT_TEST_HARNESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int weft_test_failures = 0;

#define TEST_CHECK(cond)                                                     \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
            weft_test_failures++;                                            \
        }                                                                    \
    } while (0)

#define TEST_EQ_INT(a, b)                                                    \
    do {                                                                     \
        long long va_ = (long long)(a);                                      \
        long long vb_ = (long long)(b);                                      \
        if (va_ != vb_) {                                                    \
            fprintf(stderr, "FAIL %s:%d: %s == %s (%lld != %lld)\n",         \
                    __FILE__, __LINE__, #a, #b, va_, vb_);                   \
            weft_test_failures++;                                            \
        }                                                                    \
    } while (0)

#define TEST_EQ_STR(a, b)                                                    \
    do {                                                                     \
        const char *sa_ = (a);                                               \
        const char *sb_ = (b);                                               \
        if (strcmp(sa_, sb_) != 0) {                                         \
            fprintf(stderr, "FAIL %s:%d: \"%s\" != \"%s\"\n",                \
                    __FILE__, __LINE__, sa_, sb_);                           \
            weft_test_failures++;                                            \
        }                                                                    \
    } while (0)

#define TEST_RUN(fn)                                                         \
    do {                                                                     \
        int before_ = weft_test_failures;                                    \
        fn();                                                                \
        fprintf(stderr, "%s %s\n",                                           \
                weft_test_failures == before_ ? "ok  " : "FAIL", #fn);       \
    } while (0)

#define TEST_MAIN_END() return weft_test_failures == 0 ? 0 : 1

#endif
