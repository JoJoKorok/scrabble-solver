#include "scrabble/board.h"

#include "test.h"

#include <string.h>

static ScrabbleBoardPosition position(size_t row, size_t column) {
    ScrabbleBoardPosition value = {row, column};

    return value;
}

static int creates_an_empty_board(void) {
    ScrabbleBoard *board = scrabble_board_create();
    ScrabbleBoardCell cell = {'X', 1};

    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(15, SCRABBLE_BOARD_SIZE);
    TEST_ASSERT_INT(7, SCRABBLE_BOARD_CENTER_INDEX);
    TEST_ASSERT(scrabble_board_is_empty(board));
    TEST_ASSERT_INT(0, scrabble_board_tile_count(board));
    TEST_ASSERT(scrabble_board_position_is_valid(position(0, 0)));
    TEST_ASSERT(scrabble_board_position_is_valid(position(14, 14)));
    TEST_ASSERT(scrabble_board_position_is_center(position(7, 7)));
    TEST_ASSERT(!scrabble_board_position_is_center(position(7, 8)));
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_get_cell(board, position(7, 7), &cell));
    TEST_ASSERT_INT('\0', cell.letter);
    TEST_ASSERT_INT(0, cell.is_blank);

    scrabble_board_destroy(board);
    return 0;
}

static int places_normal_and_blank_tiles(void) {
    ScrabbleBoard *board = scrabble_board_create();
    ScrabbleBoardCell cell;

    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_place_tile(board, position(7, 7), 'q', 0));
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_place_tile(board, position(0, 14), 'a', 1));
    TEST_ASSERT_INT(2, scrabble_board_tile_count(board));
    TEST_ASSERT(!scrabble_board_is_empty(board));

    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_get_cell(board, position(7, 7), &cell));
    TEST_ASSERT_INT('Q', cell.letter);
    TEST_ASSERT_INT(0, cell.is_blank);

    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_get_cell(board, position(0, 14), &cell));
    TEST_ASSERT_INT('A', cell.letter);
    TEST_ASSERT_INT(1, cell.is_blank);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_CELL_OCCUPIED,
        scrabble_board_place_tile(board, position(7, 7), 'Z', 0));

    scrabble_board_destroy(board);
    return 0;
}

static int clears_a_populated_board(void) {
    ScrabbleBoard *board = scrabble_board_create();
    ScrabbleBoardCell cell;

    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_place_tile(board, position(3, 4), 'T', 0));
    scrabble_board_clear(board);
    TEST_ASSERT(scrabble_board_is_empty(board));
    TEST_ASSERT_INT(0, scrabble_board_tile_count(board));
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_get_cell(board, position(3, 4), &cell));
    TEST_ASSERT_INT('\0', cell.letter);
    TEST_ASSERT_INT(0, cell.is_blank);

    scrabble_board_destroy(board);
    return 0;
}

static int removes_tiles_and_reports_empty_cells(void) {
    ScrabbleBoard *board = scrabble_board_create();
    ScrabbleBoardCell removed;

    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_place_tile(board, position(2, 3), 'e', 1));
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_remove_tile(board, position(2, 3), &removed));
    TEST_ASSERT_INT('E', removed.letter);
    TEST_ASSERT_INT(1, removed.is_blank);
    TEST_ASSERT(scrabble_board_is_empty(board));

    memset(&removed, 0xFF, sizeof(removed));
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_CELL_EMPTY,
        scrabble_board_remove_tile(board, position(2, 3), &removed));
    TEST_ASSERT_INT('\0', removed.letter);
    TEST_ASSERT_INT(0, removed.is_blank);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OUT_OF_BOUNDS,
        scrabble_board_remove_tile(board, position(15, 0), NULL));
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_INVALID_ARGUMENT,
        scrabble_board_remove_tile(NULL, position(0, 0), NULL));

    scrabble_board_destroy(board);
    return 0;
}

static int validates_arguments_positions_and_letters(void) {
    ScrabbleBoard *board = scrabble_board_create();
    ScrabbleBoardCell cell;

    TEST_ASSERT(board != NULL);
    TEST_ASSERT(!scrabble_board_is_empty(NULL));
    TEST_ASSERT_INT(0, scrabble_board_tile_count(NULL));
    TEST_ASSERT(!scrabble_board_position_is_valid(position(15, 0)));
    TEST_ASSERT(!scrabble_board_position_is_valid(position(0, 15)));
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_INVALID_ARGUMENT,
        scrabble_board_get_cell(NULL, position(0, 0), &cell));
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_INVALID_ARGUMENT,
        scrabble_board_get_cell(board, position(0, 0), NULL));

    memset(&cell, 0xFF, sizeof(cell));
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OUT_OF_BOUNDS,
        scrabble_board_get_cell(board, position(15, 0), &cell));
    TEST_ASSERT_INT('\0', cell.letter);
    TEST_ASSERT_INT(0, cell.is_blank);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_INVALID_ARGUMENT,
        scrabble_board_place_tile(NULL, position(0, 0), 'A', 0));
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OUT_OF_BOUNDS,
        scrabble_board_place_tile(board, position(0, 15), 'A', 0));
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_INVALID_LETTER,
        scrabble_board_place_tile(board, position(0, 0), '?', 1));
    TEST_ASSERT_INT(0, scrabble_board_tile_count(board));

    scrabble_board_clear(NULL);
    scrabble_board_destroy(NULL);
    scrabble_board_destroy(board);
    return 0;
}

static const ScrabbleTestCase TESTS[] = {
    {"creates an empty board", creates_an_empty_board},
    {"places normal and blank tiles", places_normal_and_blank_tiles},
    {"clears a populated board", clears_a_populated_board},
    {"removes tiles and reports empty cells",
     removes_tiles_and_reports_empty_cells},
    {"validates arguments, positions, and letters",
     validates_arguments_positions_and_letters}
};

TEST_MAIN(TESTS)
