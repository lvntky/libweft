#include <weft/weft.h>

#include "harness.h"

static void version_string_matches_header(void)
{
    TEST_EQ_STR(weft_version_string(), WEFT_VERSION_STRING);
}

static void version_num_matches_header(void)
{
    TEST_EQ_INT(weft_version_num(), WEFT_VERSION_NUM);
    TEST_CHECK(WEFT_VERSION_AT_LEAST(0, 1, 0));
}

static void strerror_covers_known_codes(void)
{
    TEST_EQ_STR(weft_strerror(WEFT_OK), "success");
    TEST_CHECK(weft_strerror(WEFT_ETIMEDOUT) != NULL);
    TEST_CHECK(weft_strerror(WEFT_ECANCELED) != NULL);
}

int main(void)
{
    TEST_RUN(version_string_matches_header);
    TEST_RUN(version_num_matches_header);
    TEST_RUN(strerror_covers_known_codes);
    TEST_MAIN_END();
}
