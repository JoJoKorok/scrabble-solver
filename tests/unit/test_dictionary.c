#include "scrabble/dictionary.h"

#include "test.h"
#include "test_paths.h"

static int loads_normalizes_and_deduplicates(void) {
    ScrabbleDictionaryStatus status;
    ScrabbleDictionary *dictionary = scrabble_dictionary_load(
        SCRABBLE_DICTIONARY_FIXTURE, &status);

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT_INT(SCRABBLE_DICTIONARY_OK, status);
    TEST_ASSERT_INT(5, scrabble_dictionary_count(dictionary));
    TEST_ASSERT_STRING("APPLE", scrabble_dictionary_word_at(dictionary, 0));
    TEST_ASSERT_STRING("CAT", scrabble_dictionary_word_at(dictionary, 1));
    TEST_ASSERT_STRING("DOG", scrabble_dictionary_word_at(dictionary, 2));
    TEST_ASSERT_STRING("QUIZ", scrabble_dictionary_word_at(dictionary, 3));
    TEST_ASSERT_STRING("RAIN", scrabble_dictionary_word_at(dictionary, 4));
    TEST_ASSERT(scrabble_dictionary_word_at(dictionary, 5) == NULL);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static int reports_load_errors(void) {
    ScrabbleDictionaryStatus status;

    TEST_ASSERT(scrabble_dictionary_load(NULL, &status) == NULL);
    TEST_ASSERT_INT(SCRABBLE_DICTIONARY_INVALID_ARGUMENT, status);
    TEST_ASSERT(scrabble_dictionary_load(
        "scrabble_fixture_that_does_not_exist.txt", &status) == NULL);
    TEST_ASSERT_INT(SCRABBLE_DICTIONARY_OPEN_FAILED, status);
    TEST_ASSERT_INT(0, scrabble_dictionary_count(NULL));
    TEST_ASSERT(scrabble_dictionary_word_at(NULL, 0) == NULL);
    scrabble_dictionary_destroy(NULL);
    return 0;
}

static const ScrabbleTestCase TESTS[] = {
    {"loads, normalizes, and deduplicates", loads_normalizes_and_deduplicates},
    {"reports load errors", reports_load_errors}
};

TEST_MAIN(TESTS)
