
#include "unity.h"

#include "../math.h"

// Runs before/after every test.
void setUp(void) {}
void tearDown(void) {}

static void test_clamp(void)
{

    TEST_ASSERT_EQUAL_FLOAT(3.0f, 1.0f + 2.0f);
}

// Runs the test(s)
int main(void)
{
    UnityBegin("test_math.c");

    RUN_TEST(test_clamp);

    return UnityEnd();
}