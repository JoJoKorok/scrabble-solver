#include "scrabble/placement.h"

#include "test.h"
#include "test_paths.h"

#include <string.h>

static ScrabbleBoardPosition position(size_t row, size_t column) {
    ScrabbleBoardPosition value = {row, column};

    return value;
}

static ScrabbleDictionary *load_dictionary(void) {
    ScrabbleDictionaryStatus status;
    ScrabbleDictionary *dictionary = scrabble_dictionary_load(
        SCRABBLE_SOLVER_DICTIONARY_FIXTURE, &status);

    return status == SCRABBLE_DICTIONARY_OK ? dictionary : NULL;
}

static int validates_without_changing_the_board(void) {
    ScrabbleDictionary *dictionary = load_dictionary();
    ScrabbleBoard *board = scrabble_board_create();
    ScrabbleRack rack;
    ScrabbleMove move;
    ScrabbleMove validated;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(SCRABBLE_RACK_OK,
                    scrabble_rack_init(&rack, "RETAINS"));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "RETAINS", position(7, 4), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_OK,
        scrabble_opening_move_validate(
            board, dictionary, &rack, &move, &validated));
    TEST_ASSERT(scrabble_board_is_empty(board));
    TEST_ASSERT_STRING("RETAINS", validated.word);
    TEST_ASSERT_INT(7, scrabble_move_rack_tile_count(&validated));
    TEST_ASSERT_INT(0, validated.blank_tile_mask);

    scrabble_board_destroy(board);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static int applies_a_centered_horizontal_word(void) {
    ScrabbleDictionary *dictionary = load_dictionary();
    ScrabbleBoard *board = scrabble_board_create();
    ScrabbleRack rack;
    ScrabbleMove move;
    ScrabbleMove applied;
    ScrabbleBoardCell cell;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(SCRABBLE_RACK_OK,
                    scrabble_rack_init(&rack, "RETAINS"));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "RETAINS", position(7, 4), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_OK,
        scrabble_opening_move_apply(
            board, dictionary, &rack, &move, &applied));
    TEST_ASSERT_INT(7, scrabble_board_tile_count(board));
    TEST_ASSERT_STRING("RETAINS", applied.word);

    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_get_cell(board, position(7, 4), &cell));
    TEST_ASSERT_INT('R', cell.letter);
    TEST_ASSERT_INT(0, cell.is_blank);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_get_cell(board, position(7, 7), &cell));
    TEST_ASSERT_INT('A', cell.letter);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_get_cell(board, position(7, 10), &cell));
    TEST_ASSERT_INT('S', cell.letter);
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_BOARD_NOT_EMPTY,
        scrabble_opening_move_apply(
            board, dictionary, &rack, &move, NULL));

    scrabble_board_destroy(board);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static int assigns_and_preserves_blank_tiles(void) {
    ScrabbleDictionary *dictionary = load_dictionary();
    ScrabbleBoard *board = scrabble_board_create();
    ScrabbleRack rack;
    ScrabbleMove move;
    ScrabbleMove applied;
    ScrabbleMoveTile tile;
    ScrabbleBoardCell cell;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(SCRABBLE_RACK_OK,
                    scrabble_rack_init(&rack, "RET?INS"));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "RETAINS", position(4, 7), SCRABBLE_MOVE_VERTICAL));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_OK,
        scrabble_opening_move_apply(
            board, dictionary, &rack, &move, &applied));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_tile_at(&applied, 3, &tile));
    TEST_ASSERT_INT('A', tile.letter);
    TEST_ASSERT_INT(1, tile.is_blank);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_get_cell(board, position(7, 7), &cell));
    TEST_ASSERT_INT('A', cell.letter);
    TEST_ASSERT_INT(1, cell.is_blank);

    scrabble_board_clear(board);
    TEST_ASSERT_INT(SCRABBLE_RACK_OK,
                    scrabble_rack_init(&rack, "LE?TERS"));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "LETTERS", position(7, 4), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_set_tile_blank(&move, 2, 1));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_OK,
        scrabble_opening_move_validate(
            board, dictionary, &rack, &move, &applied));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_tile_at(&applied, 2, &tile));
    TEST_ASSERT_INT(1, tile.is_blank);
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_tile_at(&applied, 3, &tile));
    TEST_ASSERT_INT(0, tile.is_blank);

    scrabble_board_destroy(board);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static int rejects_invalid_opening_moves_atomically(void) {
    ScrabbleDictionary *dictionary = load_dictionary();
    ScrabbleBoard *board = scrabble_board_create();
    ScrabbleRack rack;
    ScrabbleMove move;
    ScrabbleMove output;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(SCRABBLE_RACK_OK,
                    scrabble_rack_init(&rack, "RETAINS"));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "RETAINS", position(0, 0), SCRABBLE_MOVE_HORIZONTAL));
    memset(&output, 0xFF, sizeof(output));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_MOVE_NOT_CENTERED,
        scrabble_opening_move_apply(
            board, dictionary, &rack, &move, &output));
    TEST_ASSERT(scrabble_board_is_empty(board));
    TEST_ASSERT_INT(0, output.length);

    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "ABCDEFG", position(7, 4), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_WORD_NOT_IN_DICTIONARY,
        scrabble_opening_move_validate(
            board, dictionary, &rack, &move, NULL));

    TEST_ASSERT_INT(SCRABBLE_RACK_OK,
                    scrabble_rack_init(&rack, "A"));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "A", position(7, 7), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INVALID_MOVE,
        scrabble_opening_move_validate(
            board, dictionary, &rack, &move, NULL));

    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "RETAINS", position(7, 4), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(SCRABBLE_RACK_OK,
                    scrabble_rack_init(&rack, "XXXXXXX"));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_RACK_MISMATCH,
        scrabble_opening_move_validate(
            board, dictionary, &rack, &move, NULL));

    TEST_ASSERT_INT(SCRABBLE_RACK_OK,
                    scrabble_rack_init(&rack, "RETAINS"));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_set_tile_from_rack(&move, 0, 0));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INVALID_MOVE,
        scrabble_opening_move_validate(
            board, dictionary, &rack, &move, NULL));
    TEST_ASSERT(scrabble_board_is_empty(board));

    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INVALID_ARGUMENT,
        scrabble_opening_move_validate(
            NULL, dictionary, &rack, &move, NULL));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INVALID_ARGUMENT,
        scrabble_opening_move_validate(
            board, NULL, &rack, &move, NULL));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INVALID_ARGUMENT,
        scrabble_opening_move_validate(
            board, dictionary, NULL, &move, NULL));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INVALID_ARGUMENT,
        scrabble_opening_move_validate(
            board, dictionary, &rack, NULL, NULL));

    scrabble_board_destroy(board);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static const ScrabbleTestCase TESTS[] = {
    {"validates without changing the board",
     validates_without_changing_the_board},
    {"applies a centered horizontal word",
     applies_a_centered_horizontal_word},
    {"assigns and preserves blank tiles",
     assigns_and_preserves_blank_tiles},
    {"rejects invalid opening moves atomically",
     rejects_invalid_opening_moves_atomically}
};

TEST_MAIN(TESTS)
