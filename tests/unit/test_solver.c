#include "scrabble/dictionary.h"
#include "scrabble/rack.h"
#include "scrabble/solver.h"

#include "test.h"
#include "test_paths.h"

static ScrabbleDictionary *load_fixture(void) {
    ScrabbleDictionaryStatus status;
    ScrabbleDictionary *dictionary = scrabble_dictionary_load(
        SCRABBLE_SOLVER_DICTIONARY_FIXTURE, &status);

    return status == SCRABBLE_DICTIONARY_OK ? dictionary : NULL;
}

static int ranks_results_by_score_length_and_name(void) {
    ScrabbleDictionary *dictionary = load_fixture();
    ScrabbleRack rack;
    ScrabbleResultSet results;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT_INT(SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "RET?INS"));
    TEST_ASSERT_INT(SCRABBLE_SOLVER_OK,
                    scrabble_solve(dictionary, &rack, &results));
    TEST_ASSERT_INT(7, results.count);
    TEST_ASSERT_STRING("RETAINS", results.items[0].word);
    TEST_ASSERT_INT(6, results.items[0].score);
    TEST_ASSERT_STRING("RETAIN", results.items[1].word);
    TEST_ASSERT_STRING("STRAIN", results.items[2].word);
    TEST_ASSERT_STRING("TRAINS", results.items[3].word);
    TEST_ASSERT_INT(5, results.items[1].score);
    TEST_ASSERT_INT(5, results.items[2].score);
    TEST_ASSERT_INT(5, results.items[3].score);
    TEST_ASSERT_INT(1, results.items[0].rack_match.blanks_used);
    scrabble_result_set_destroy(&results);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static int returns_an_empty_owned_result_set(void) {
    ScrabbleDictionary *dictionary = load_fixture();
    ScrabbleRack rack;
    ScrabbleResultSet results = {(ScrabbleResult *)1, 99};

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT_INT(SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "XXXXXXX"));
    TEST_ASSERT_INT(SCRABBLE_SOLVER_OK,
                    scrabble_solve(dictionary, &rack, &results));
    TEST_ASSERT(results.items == NULL);
    TEST_ASSERT_INT(0, results.count);
    scrabble_result_set_destroy(&results);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static int validates_solver_arguments(void) {
    ScrabbleResultSet results = {(ScrabbleResult *)1, 99};
    ScrabbleRack rack;

    TEST_ASSERT_INT(SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "LETTERS"));
    TEST_ASSERT_INT(SCRABBLE_SOLVER_INVALID_ARGUMENT,
                    scrabble_solve(NULL, &rack, &results));
    TEST_ASSERT(results.items == NULL);
    TEST_ASSERT_INT(0, results.count);
    TEST_ASSERT_INT(SCRABBLE_SOLVER_INVALID_ARGUMENT,
                    scrabble_solve(NULL, &rack, NULL));
    scrabble_result_set_destroy(NULL);
    return 0;
}

static const ScrabbleTestCase TESTS[] = {
    {"ranks results by score, length, and name",
     ranks_results_by_score_length_and_name},
    {"returns an empty owned result set", returns_an_empty_owned_result_set},
    {"validates solver arguments", validates_solver_arguments}
};

TEST_MAIN(TESTS)
