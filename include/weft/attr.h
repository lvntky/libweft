#ifndef WEFT_ATTR_H
#define WEFT_ATTR_H

#if defined(__GNUC__) || defined(__clang__)
#  define WEFT_NODISCARD    __attribute__((warn_unused_result))
#  define WEFT_NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#  define WEFT_PRINTF(f, a) __attribute__((format(printf, f, a)))
#  define WEFT_MALLOC       __attribute__((malloc))
#  define WEFT_PURE         __attribute__((pure))
#  define WEFT_NORETURN     __attribute__((noreturn))
#else
#  define WEFT_NODISCARD
#  define WEFT_NONNULL(...)
#  define WEFT_PRINTF(f, a)
#  define WEFT_MALLOC
#  define WEFT_PURE
#  define WEFT_NORETURN
#endif

#endif
