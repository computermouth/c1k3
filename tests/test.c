
#include "unity.h"

#include "../math.h"

// Runs before/after every test.
void setUp(void) {}
void tearDown(void) {}

static void test_clamp(void)
{
    TEST_ASSERT_EQUAL_FLOAT(clamp(1.5, 2.0, 3.0), 2.0);
    TEST_ASSERT_EQUAL_FLOAT(clamp(2.5, 2.0, 3.0), 2.5);
    TEST_ASSERT_EQUAL_FLOAT(clamp(3.5, 2.0, 3.0), 3.0);
}

// Runs the test(s)
int main(void)
{
    UnityBegin("test_math.c");

    RUN_TEST(test_clamp);

    return UnityEnd();
}