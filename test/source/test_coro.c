#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "internal.h"

#include "weft_test_util.h"
#include <fenv.h>

static void *main_sp;

static void resume(weft__coro *c)
{
    c->ret = &main_sp;
    weft__coro_switch(&main_sp, c->sp);
}

static void coro_yield(weft__coro *c)
{
    weft__coro_switch(&c->sp, *c->ret);
}

void weft_test_resume(weft__coro *c);

void weft_test_resume(weft__coro *c)
{
    resume(c);
}

static int ran;
static void *seen_arg;

static void set_ran(void *arg)
{
    ran = 1;
    seen_arg = arg;
}

static void new_does_not_run(void)
{
    ran = 0;
    weft__coro *c = weft__coro_new(set_ran, NULL, 0);
    TEST_CHECK(c != NULL);
    TEST_EQ_INT(ran, 0);
    TEST_EQ_INT(c->state, WEFT__CORO_NEW);
    weft__coro_free(c);
}

static void runs_to_completion(void)
{
    ran = 0;
    weft__coro *c = weft__coro_new(set_ran, NULL, 0);
    resume(c);
    TEST_EQ_INT(ran, 1);
    TEST_EQ_INT(c->state, WEFT__CORO_DONE);
    weft__coro_free(c);
}

static void argument_is_passed(void)
{
    int marker = 0;
    seen_arg = NULL;
    weft__coro *c = weft__coro_new(set_ran, &marker, 0);
    resume(c);
    TEST_CHECK(seen_arg == &marker);
    weft__coro_free(c);
}

static int locals_ok;

static void locals_body(void *arg)
{
    weft__coro *c = arg;
    volatile long a = 0x1111;
    volatile long b = 0x2222;
    char buf[512];

    memset(buf, 0x5a, sizeof buf);
    coro_yield(c);
    locals_ok = (a == 0x1111) && (b == 0x2222) && (buf[0] == 0x5a) && (buf[511] == 0x5a);
    coro_yield(c);
    locals_ok = locals_ok && (a == 0x1111) && (b == 0x2222);
}

static void locals_survive_yield(void)
{
    locals_ok = 0;
    weft__coro *c = weft__coro_new(locals_body, NULL, 0);
    c->arg = c;
    resume(c);
    TEST_EQ_INT(c->state, WEFT__CORO_BLOCKED);
    resume(c);
    TEST_EQ_INT(locals_ok, 1);
    resume(c);
    TEST_EQ_INT(locals_ok, 1);
    TEST_EQ_INT(c->state, WEFT__CORO_DONE);
    weft__coro_free(c);
}

#define PING_PONG_N 1000000

static long pong_count;

static void pong_body(void *arg)
{
    weft__coro *c = arg;
    for (long i = 0; i < PING_PONG_N; i++) {
        pong_count++;
        coro_yield(c);
    }
}

static void ping_pong_one_million(void)
{
    pong_count = 0;
    weft__coro *c = weft__coro_new(pong_body, NULL, 0);
    c->arg = c;
    for (long i = 0; i < PING_PONG_N; i++) {
        resume(c);
    }
    TEST_EQ_INT(pong_count, PING_PONG_N);
    resume(c);
    TEST_EQ_INT(c->state, WEFT__CORO_DONE);
    weft__coro_free(c);
}

#define ROUND_ROBIN_N     64
#define ROUND_ROBIN_YIELD 100

static long rr_counts[ROUND_ROBIN_N];

struct rr_ctx {
    weft__coro *self;
    int index;
};

static void rr_body(void *arg)
{
    struct rr_ctx *ctx = arg;
    for (int i = 0; i < ROUND_ROBIN_YIELD; i++) {
        rr_counts[ctx->index]++;
        coro_yield(ctx->self);
    }
}

static void interleaves_many_coroutines(void)
{
    weft__coro *cs[ROUND_ROBIN_N];
    struct rr_ctx ctxs[ROUND_ROBIN_N];

    memset(rr_counts, 0, sizeof rr_counts);

    for (int i = 0; i < ROUND_ROBIN_N; i++) {
        ctxs[i].index = i;
        cs[i] = weft__coro_new(rr_body, &ctxs[i], 0);
        ctxs[i].self = cs[i];
    }

    for (int round = 0; round <= ROUND_ROBIN_YIELD; round++) {
        for (int i = 0; i < ROUND_ROBIN_N; i++) {
            if (cs[i]->state != WEFT__CORO_DONE) {
                resume(cs[i]);
            }
        }
    }

    for (int i = 0; i < ROUND_ROBIN_N; i++) {
        TEST_EQ_INT(rr_counts[i], ROUND_ROBIN_YIELD);
        TEST_EQ_INT(cs[i]->state, WEFT__CORO_DONE);
        weft__coro_free(cs[i]);
    }
}

static int align_ok;
static int sse_ok;

static void align_body(void *arg)
{
    weft__coro *c = arg;
    _Alignas(16) char probe[16];
    char buf[64];

    align_ok = ((uintptr_t)probe % 16) == 0;
    snprintf(buf, sizeof buf, "%.3f", 1.5);
    sse_ok = strcmp(buf, "1.500") == 0;
    coro_yield(c);

    snprintf(buf, sizeof buf, "%.3f", 2.5);
    sse_ok = sse_ok && (strcmp(buf, "2.500") == 0);
}

static void stack_is_abi_aligned(void)
{
    align_ok = 0;
    sse_ok = 0;
    weft__coro *c = weft__coro_new(align_body, NULL, 0);
    c->arg = c;
    resume(c);
    TEST_EQ_INT(align_ok, 1);
    TEST_EQ_INT(sse_ok, 1);
    resume(c);
    TEST_EQ_INT(sse_ok, 1);
    weft__coro_free(c);
}

static long recurse(long depth, weft__coro *c)
{
    volatile char pad[256];
    pad[0] = (char)depth;
    if (depth == 0) {
        coro_yield(c);
        return 0;
    }
    return pad[0] + recurse(depth - 1, c) - pad[0];
}

static int recursion_ok;

static void recursion_body(void *arg)
{
    weft__coro *c = arg;
    recursion_ok = recurse(64, c) == 0;
}

static void deep_recursion_within_stack(void)
{
    recursion_ok = 0;
    weft__coro *c = weft__coro_new(recursion_body, NULL, 256 * 1024);
    c->arg = c;
    resume(c);
    resume(c);
    TEST_EQ_INT(recursion_ok, 1);
    TEST_EQ_INT(c->state, WEFT__CORO_DONE);
    weft__coro_free(c);
}

static int fpu_ok;

static void fpu_body(void *arg)
{
    weft__coro *c = arg;
    fesetround(FE_TOWARDZERO);
    coro_yield(c);
    fpu_ok = fegetround() == FE_TOWARDZERO;
    fesetround(FE_TONEAREST);
}

static void fpu_state_is_per_coroutine(void)
{
    fpu_ok = 0;
    int outer_before = fegetround();

    weft__coro *c = weft__coro_new(fpu_body, NULL, 0);
    c->arg = c;
    resume(c);
    TEST_EQ_INT(fegetround(), outer_before);
    resume(c);
    TEST_EQ_INT(fpu_ok, 1);
    TEST_EQ_INT(fegetround(), outer_before);
    weft__coro_free(c);
}

#if defined(__x86_64__)

unsigned long cs_regs[5];

static void cs_clobber(void *arg)
{
    (void)arg;
    __asm__ volatile("movabs $0xdeadbeefdeadbeef, %%rbx\n\t"
                     "movabs $0xdeadbeefdeadbeef, %%r12\n\t"
                     "movabs $0xdeadbeefdeadbeef, %%r13\n\t"
                     "movabs $0xdeadbeefdeadbeef, %%r14\n\t"
                     "movabs $0xdeadbeefdeadbeef, %%r15\n\t"
                     :
                     :
                     : "rbx", "r12", "r13", "r14", "r15");
}

static void callee_saved_survive_switch(void)
{
    weft__coro *c = weft__coro_new(cs_clobber, NULL, 0);

    memset(cs_regs, 0, sizeof cs_regs);

    __asm__ volatile("movabs $0x1111111111111111, %%rbx\n\t"
                     "movabs $0x2222222222222222, %%r12\n\t"
                     "movabs $0x3333333333333333, %%r13\n\t"
                     "movabs $0x4444444444444444, %%r14\n\t"
                     "movabs $0x5555555555555555, %%r15\n\t"
                     "call weft_test_resume\n\t"
                     "lea cs_regs(%%rip), %%rax\n\t"
                     "mov %%rbx, 0(%%rax)\n\t"
                     "mov %%r12, 8(%%rax)\n\t"
                     "mov %%r13, 16(%%rax)\n\t"
                     "mov %%r14, 24(%%rax)\n\t"
                     "mov %%r15, 32(%%rax)\n\t"
                     :
                     : "D"(c)
                     : "rax", "rbx", "r12", "r13", "r14", "r15", "rcx", "rdx", "rsi", "r8", "r9",
                       "r10", "r11", "memory", "cc");

    TEST_CHECK(cs_regs[0] == 0x1111111111111111UL);
    TEST_CHECK(cs_regs[1] == 0x2222222222222222UL);
    TEST_CHECK(cs_regs[2] == 0x3333333333333333UL);
    TEST_CHECK(cs_regs[3] == 0x4444444444444444UL);
    TEST_CHECK(cs_regs[4] == 0x5555555555555555UL);

    weft__coro_free(c);
}

#else

static void callee_saved_survive_switch(void)
{}

#endif

static void noop(void *arg)
{
    (void)arg;
}

static void create_run_free_repeatedly(void)
{
    for (int i = 0; i < 10000; i++) {
        weft__coro *c = weft__coro_new(noop, NULL, 0);
        if (c == NULL) {
            TEST_CHECK(c != NULL);
            return;
        }
        resume(c);
        weft__coro_free(c);
    }
    TEST_CHECK(1);
}

static void abandoned_coroutine_frees_cleanly(void)
{
    weft__coro *c = weft__coro_new(locals_body, NULL, 0);
    c->arg = c;
    resume(c);
    TEST_EQ_INT(c->state, WEFT__CORO_BLOCKED);
    weft__coro_free(c);
}

int main(void)
{
    TEST_RUN(new_does_not_run);
    TEST_RUN(runs_to_completion);
    TEST_RUN(argument_is_passed);
    TEST_RUN(locals_survive_yield);
    TEST_RUN(stack_is_abi_aligned);
    TEST_RUN(callee_saved_survive_switch);
    TEST_RUN(fpu_state_is_per_coroutine);
    TEST_RUN(deep_recursion_within_stack);
    TEST_RUN(interleaves_many_coroutines);
    TEST_RUN(create_run_free_repeatedly);
    TEST_RUN(abandoned_coroutine_frees_cleanly);
    TEST_RUN(ping_pong_one_million);
    TEST_MAIN_END();
}
