#ifndef SCRABBLE_TEST_H
#define SCRABBLE_TEST_H

#include <stdio.h>
#include <string.h>

typedef int (*ScrabbleTestFunction)(void);

typedef struct {
    const char *name;
    ScrabbleTestFunction function;
} ScrabbleTestCase;

#define TEST_ASSERT(condition)                                                \
    do {                                                                      \
        if (!(condition)) {                                                   \
            fprintf(stderr, "%s:%d: assertion failed: %s\n",                \
                    __FILE__, __LINE__, #condition);                          \
            return 1;                                                         \
        }                                                                     \
    } while (0)

#define TEST_ASSERT_INT(expected, actual)                                     \
    do {                                                                      \
        long test_expected = (long)(expected);                                \
        long test_actual = (long)(actual);                                    \
        if (test_expected != test_actual) {                                   \
            fprintf(stderr, "%s:%d: expected %ld, got %ld\n",               \
                    __FILE__, __LINE__, test_expected, test_actual);           \
            return 1;                                                         \
        }                                                                     \
    } while (0)

#define TEST_ASSERT_STRING(expected, actual)                                  \
    do {                                                                      \
        const char *test_expected = (expected);                               \
        const char *test_actual = (actual);                                   \
        if (test_expected == NULL || test_actual == NULL ||                   \
            strcmp(test_expected, test_actual) != 0) {                        \
            fprintf(stderr, "%s:%d: expected \"%s\", got \"%s\"\n",         \
                    __FILE__, __LINE__,                                        \
                    test_expected == NULL ? "(null)" : test_expected,         \
                    test_actual == NULL ? "(null)" : test_actual);            \
            return 1;                                                         \
        }                                                                     \
    } while (0)

static int scrabble_run_tests(const ScrabbleTestCase *tests, size_t count) {
    size_t failures = 0;

    for (size_t index = 0; index < count; ++index) {
        int result = tests[index].function();

        if (result == 0) {
            printf("PASS %s\n", tests[index].name);
        } else {
            fprintf(stderr, "FAIL %s\n", tests[index].name);
            ++failures;
        }
    }

    printf("%zu tests, %zu failures\n", count, failures);
    return failures == 0 ? 0 : 1;
}

#define TEST_MAIN(test_cases)                                                 \
    int main(void) {                                                          \
        return scrabble_run_tests(                                            \
            (test_cases), sizeof(test_cases) / sizeof((test_cases)[0]));       \
    }

#endif
