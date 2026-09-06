#include "scrabble/dictionary.h"
#include "scrabble/placement.h"
#include "scrabble/rack.h"
#include "scrabble/scoring.h"
#include "scrabble/solver.h"

#include "test.h"
#include "test_paths.h"

static ScrabbleDictionary *load_fixture(void) {
    ScrabbleDictionaryStatus status;
    ScrabbleDictionary *dictionary = scrabble_dictionary_load(
        SCRABBLE_SOLVER_DICTIONARY_FIXTURE, &status);

    return status == SCRABBLE_DICTIONARY_OK ? dictionary : NULL;
}

static ScrabbleBoardPosition position(size_t row, size_t column) {
    ScrabbleBoardPosition value = {row, column};

    return value;
}

static const ScrabbleResult *find_placement(
    const ScrabbleResultSet *results,
    const char *word,
    ScrabbleBoardPosition start,
    ScrabbleMoveDirection direction) {
    for (size_t index = 0; index < results->count; ++index) {
        const ScrabbleResult *result = &results->items[index];

        if (result->has_placement &&
            result->move.start.row == start.row &&
            result->move.start.column == start.column &&
            result->move.direction == direction &&
            strcmp(result->word, word) == 0) {
            return result;
        }
    }

    return NULL;
}

static ScrabbleBoard *create_cross_word_board(void) {
    ScrabbleBoard *board = scrabble_board_create();

    if (board == NULL ||
        scrabble_board_place_tile(board, position(7, 4), 'R', 0) !=
            SCRABBLE_BOARD_OK ||
        scrabble_board_place_tile(board, position(7, 5), 'A', 0) !=
            SCRABBLE_BOARD_OK ||
        scrabble_board_place_tile(board, position(7, 6), 'I', 0) !=
            SCRABBLE_BOARD_OK) {
        scrabble_board_destroy(board);
        return NULL;
    }

    return board;
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
    TEST_ASSERT_INT(0, results.items[0].has_placement);
    scrabble_result_set_destroy(&results);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static int finds_ranks_and_revalidates_board_placements(void) {
    ScrabbleDictionary *dictionary = load_fixture();
    ScrabbleBoard *board = create_cross_word_board();
    ScrabbleRack rack;
    ScrabbleResultSet results;
    const ScrabbleResult *train;
    size_t expected_count = 0;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(
        SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "TRAIN"));
    TEST_ASSERT_INT(
        SCRABBLE_SOLVER_OK,
        scrabble_solve_board(dictionary, &rack, board, &results));
    TEST_ASSERT(results.count > 0);

    train = find_placement(
        &results,
        "TRAIN",
        position(3, 7),
        SCRABBLE_MOVE_VERTICAL);
    TEST_ASSERT(train != NULL);
    TEST_ASSERT_INT(20, train->score);
    TEST_ASSERT_INT(5, scrabble_move_rack_tile_count(&train->move));
    TEST_ASSERT_INT(0, train->rack_match.blanks_used);
    TEST_ASSERT_INT(3, scrabble_board_tile_count(board));

    for (size_t word_index = 0;
         word_index < scrabble_dictionary_count(dictionary);
         ++word_index) {
        const char *word = scrabble_dictionary_word_at(
            dictionary, word_index);

        for (int direction = SCRABBLE_MOVE_HORIZONTAL;
             direction <= SCRABBLE_MOVE_VERTICAL;
             ++direction) {
            for (size_t row = 0; row < SCRABBLE_BOARD_SIZE; ++row) {
                for (size_t column = 0;
                     column < SCRABBLE_BOARD_SIZE;
                     ++column) {
                    ScrabbleBoardPosition start = {row, column};
                    ScrabbleMove proposal;

                    if (scrabble_move_init(
                            &proposal,
                            word,
                            start,
                            (ScrabbleMoveDirection)direction) ==
                            SCRABBLE_MOVE_OK &&
                        scrabble_connected_move_validate(
                            board,
                            dictionary,
                            &rack,
                            &proposal,
                            NULL) == SCRABBLE_PLACEMENT_OK) {
                        ++expected_count;
                        TEST_ASSERT(find_placement(
                            &results,
                            word,
                            start,
                            (ScrabbleMoveDirection)direction) != NULL);
                    }
                }
            }
        }
    }
    TEST_ASSERT_INT(expected_count, results.count);

    for (size_t index = 0; index < results.count; ++index) {
        ScrabbleMove validated;
        ScrabbleMoveScore score;

        TEST_ASSERT_INT(1, results.items[index].has_placement);
        TEST_ASSERT_INT(
            SCRABBLE_PLACEMENT_OK,
            scrabble_connected_move_validate(
                board,
                dictionary,
                &rack,
                &results.items[index].move,
                &validated));
        TEST_ASSERT_INT(
            SCRABBLE_SCORING_OK,
            scrabble_score_move(board, &validated, &score));
        TEST_ASSERT_INT(score.total_score, results.items[index].score);
        if (index > 0) {
            TEST_ASSERT(results.items[index - 1].score >=
                        results.items[index].score);
        }
    }

    scrabble_result_set_destroy(&results);
    scrabble_board_destroy(board);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static int chooses_the_highest_scoring_blank_positions(void) {
    ScrabbleDictionary *dictionary = load_fixture();
    ScrabbleBoard *board = scrabble_board_create();
    ScrabbleRack rack;
    ScrabbleResultSet results;
    const ScrabbleResult *letters;
    ScrabbleMoveTile tile;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_place_tile(board, position(7, 5), 'R', 0));
    TEST_ASSERT_INT(
        SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "LETES?"));
    TEST_ASSERT_INT(
        SCRABBLE_SOLVER_OK,
        scrabble_solve_board(dictionary, &rack, board, &results));

    letters = find_placement(
        &results,
        "LETTERS",
        position(7, 0),
        SCRABBLE_MOVE_HORIZONTAL);
    TEST_ASSERT(letters != NULL);
    TEST_ASSERT_INT(21, letters->score);
    TEST_ASSERT_INT(1, letters->rack_match.blanks_used);
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_tile_at(&letters->move, 2, &tile));
    TEST_ASSERT_INT(1, tile.is_blank);
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_tile_at(&letters->move, 3, &tile));
    TEST_ASSERT_INT(0, tile.is_blank);

    scrabble_result_set_destroy(&results);
    scrabble_board_destroy(board);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static int handles_empty_boards_no_moves_and_invalid_arguments(void) {
    ScrabbleDictionary *dictionary = load_fixture();
    ScrabbleBoard *board = scrabble_board_create();
    ScrabbleRack rack;
    ScrabbleResultSet results = {(ScrabbleResult *)1, 99};

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(
        SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "TRAIN"));
    TEST_ASSERT_INT(
        SCRABBLE_SOLVER_OK,
        scrabble_solve_board(dictionary, &rack, board, &results));
    TEST_ASSERT(results.items == NULL);
    TEST_ASSERT_INT(0, results.count);

    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_place_tile(board, position(7, 7), 'A', 0));
    TEST_ASSERT_INT(
        SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "XXXXXXX"));
    TEST_ASSERT_INT(
        SCRABBLE_SOLVER_OK,
        scrabble_solve_board(dictionary, &rack, board, &results));
    TEST_ASSERT(results.items == NULL);
    TEST_ASSERT_INT(0, results.count);

    results.items = (ScrabbleResult *)1;
    results.count = 99;
    TEST_ASSERT_INT(
        SCRABBLE_SOLVER_INVALID_ARGUMENT,
        scrabble_solve_board(NULL, &rack, board, &results));
    TEST_ASSERT(results.items == NULL);
    TEST_ASSERT_INT(0, results.count);
    TEST_ASSERT_INT(
        SCRABBLE_SOLVER_INVALID_ARGUMENT,
        scrabble_solve_board(dictionary, NULL, board, &results));
    TEST_ASSERT_INT(
        SCRABBLE_SOLVER_INVALID_ARGUMENT,
        scrabble_solve_board(dictionary, &rack, NULL, &results));
    TEST_ASSERT_INT(
        SCRABBLE_SOLVER_INVALID_ARGUMENT,
        scrabble_solve_board(dictionary, &rack, board, NULL));

    scrabble_board_destroy(board);
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
    {"finds, ranks, and revalidates board placements",
     finds_ranks_and_revalidates_board_placements},
    {"chooses the highest-scoring blank positions",
     chooses_the_highest_scoring_blank_positions},
    {"handles empty boards, no moves, and invalid arguments",
     handles_empty_boards_no_moves_and_invalid_arguments},
    {"validates solver arguments", validates_solver_arguments}
};

TEST_MAIN(TESTS)
