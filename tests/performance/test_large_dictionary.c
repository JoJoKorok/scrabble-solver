#include "scrabble/dictionary.h"
#include "scrabble/rack.h"
#include "scrabble/solver.h"

#include "dictionary_internal.h"
#include "test.h"
#include "test_paths.h"

#include <string.h>
#include <time.h>

enum {
    SCRABBLE_PERFORMANCE_ROUNDS = 25
};

typedef struct {
    const char *rack;
    const char *expected_word;
} RackScenario;

static ScrabbleBoardPosition position(size_t row, size_t column) {
    ScrabbleBoardPosition value = {row, column};

    return value;
}

static int result_set_contains(
    const ScrabbleResultSet *results,
    const char *word) {
    for (size_t index = 0; index < results->count; ++index) {
        if (strcmp(results->items[index].word, word) == 0) {
            return 1;
        }
    }

    return 0;
}

static int repeatedly_solves_full_dictionary(void) {
    static const RackScenario scenarios[] = {
        {"EEOIFNX", "FIX"},
        {"RETAINS", "RETAINS"},
        {"QUIZZES", "QUIZZES"},
        {"READING", "READING"}
    };
    ScrabbleDictionaryStatus dictionary_status;
    ScrabbleDictionary *dictionary = scrabble_dictionary_load(
        SCRABBLE_BUNDLED_DICTIONARY_FIXTURE, &dictionary_status);
    clock_t started_at;
    clock_t finished_at;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT_INT(SCRABBLE_DICTIONARY_OK, dictionary_status);
    TEST_ASSERT_INT(169266, scrabble_dictionary_count(dictionary));
    TEST_ASSERT_INT(52163,
                    scrabble_dictionary_candidate_count(dictionary, 7));
    TEST_ASSERT(scrabble_dictionary_candidate_count(dictionary, 7) <
                scrabble_dictionary_count(dictionary) / 2);

    started_at = clock();
    for (size_t round = 0; round < SCRABBLE_PERFORMANCE_ROUNDS; ++round) {
        for (size_t scenario_index = 0;
             scenario_index < sizeof(scenarios) / sizeof(scenarios[0]);
             ++scenario_index) {
            ScrabbleRack rack;
            ScrabbleResultSet results;

            TEST_ASSERT_INT(
                SCRABBLE_RACK_OK,
                scrabble_rack_init(&rack, scenarios[scenario_index].rack));
            TEST_ASSERT_INT(
                SCRABBLE_SOLVER_OK,
                scrabble_solve(dictionary, &rack, &results));
            TEST_ASSERT(result_set_contains(
                &results, scenarios[scenario_index].expected_word));
            scrabble_result_set_destroy(&results);
        }
    }
    finished_at = clock();

    printf("Solved %d full-dictionary racks in %.3f CPU seconds\n",
           SCRABBLE_PERFORMANCE_ROUNDS *
               (int)(sizeof(scenarios) / sizeof(scenarios[0])),
           (double)(finished_at - started_at) / CLOCKS_PER_SEC);

    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static int solves_a_full_dictionary_board(void) {
    const char *opening = "RETAINS";
    ScrabbleDictionaryStatus dictionary_status;
    ScrabbleDictionary *dictionary = scrabble_dictionary_load(
        SCRABBLE_BUNDLED_DICTIONARY_FIXTURE, &dictionary_status);
    ScrabbleBoard *board = scrabble_board_create();
    ScrabbleRack rack;
    ScrabbleResultSet results;
    clock_t started_at;
    clock_t finished_at;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT_INT(SCRABBLE_DICTIONARY_OK, dictionary_status);
    TEST_ASSERT(board != NULL);
    for (size_t index = 0; opening[index] != '\0'; ++index) {
        TEST_ASSERT_INT(
            SCRABBLE_BOARD_OK,
            scrabble_board_place_tile(
                board, position(7, 4 + index), opening[index], 0));
    }
    TEST_ASSERT_INT(SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "RIN"));

    started_at = clock();
    TEST_ASSERT_INT(
        SCRABBLE_SOLVER_OK,
        scrabble_solve_board(dictionary, &rack, board, &results));
    finished_at = clock();
    TEST_ASSERT(result_set_contains(&results, "RAIN"));

    printf(
        "Found %zu legal board moves in %.3f CPU seconds\n",
        results.count,
        (double)(finished_at - started_at) / CLOCKS_PER_SEC);

    scrabble_result_set_destroy(&results);
    scrabble_board_destroy(board);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static const ScrabbleTestCase TESTS[] = {
    {"repeatedly solves the full dictionary",
     repeatedly_solves_full_dictionary},
    {"solves a full-dictionary board",
     solves_a_full_dictionary_board}
};

TEST_MAIN(TESTS)
