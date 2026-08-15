#include "scrabble/move.h"

#include "test.h"

#include <string.h>

static ScrabbleBoardPosition position(size_t row, size_t column) {
    ScrabbleBoardPosition value = {row, column};

    return value;
}

static int initializes_horizontal_move_geometry(void) {
    ScrabbleMove move;
    ScrabbleMoveTile tile;

    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "retains", position(7, 4), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_STRING("RETAINS", move.word);
    TEST_ASSERT_INT(7, move.length);
    TEST_ASSERT_INT(7, scrabble_move_rack_tile_count(&move));
    TEST_ASSERT_INT(SCRABBLE_MOVE_HORIZONTAL, move.direction);
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_tile_at(&move, 0, &tile));
    TEST_ASSERT_INT(7, tile.position.row);
    TEST_ASSERT_INT(4, tile.position.column);
    TEST_ASSERT_INT('R', tile.letter);
    TEST_ASSERT_INT(1, tile.from_rack);
    TEST_ASSERT_INT(0, tile.is_blank);
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_tile_at(&move, 6, &tile));
    TEST_ASSERT_INT(7, tile.position.row);
    TEST_ASSERT_INT(10, tile.position.column);
    TEST_ASSERT_INT('S', tile.letter);
    TEST_ASSERT(scrabble_move_covers_position(&move, position(7, 7)));
    TEST_ASSERT(!scrabble_move_covers_position(&move, position(8, 7)));
    return 0;
}

static int tracks_intersections_and_blank_tiles(void) {
    ScrabbleMove move;
    ScrabbleMoveTile tile;

    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "READ", position(5, 7), SCRABBLE_MOVE_VERTICAL));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_set_tile_from_rack(&move, 1, 0));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_set_tile_blank(&move, 2, 1));
    TEST_ASSERT_INT(3, scrabble_move_rack_tile_count(&move));

    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_tile_at(&move, 1, &tile));
    TEST_ASSERT_INT(6, tile.position.row);
    TEST_ASSERT_INT(7, tile.position.column);
    TEST_ASSERT_INT('E', tile.letter);
    TEST_ASSERT_INT(0, tile.from_rack);
    TEST_ASSERT_INT(0, tile.is_blank);

    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_tile_at(&move, 2, &tile));
    TEST_ASSERT_INT(7, tile.position.row);
    TEST_ASSERT_INT('A', tile.letter);
    TEST_ASSERT_INT(1, tile.from_rack);
    TEST_ASSERT_INT(1, tile.is_blank);
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_TILE_NOT_FROM_RACK,
        scrabble_move_set_tile_blank(&move, 1, 1));

    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_set_tile_from_rack(&move, 2, 0));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_tile_at(&move, 2, &tile));
    TEST_ASSERT_INT(0, tile.from_rack);
    TEST_ASSERT_INT(0, tile.is_blank);
    return 0;
}

static int accepts_moves_that_end_at_board_edges(void) {
    ScrabbleMove move;
    ScrabbleMoveTile tile;

    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "CAT", position(14, 12), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_tile_at(&move, 2, &tile));
    TEST_ASSERT_INT(14, tile.position.row);
    TEST_ASSERT_INT(14, tile.position.column);

    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "DOG", position(12, 14), SCRABBLE_MOVE_VERTICAL));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_tile_at(&move, 2, &tile));
    TEST_ASSERT_INT(14, tile.position.row);
    TEST_ASSERT_INT(14, tile.position.column);
    return 0;
}

static int rejects_invalid_moves_and_tile_indexes(void) {
    ScrabbleMove move;
    ScrabbleMoveTile tile;

    memset(&move, 0xFF, sizeof(move));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_INVALID_ARGUMENT,
        scrabble_move_init(NULL, "CAT", position(0, 0),
                           SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_INVALID_ARGUMENT,
        scrabble_move_init(&move, NULL, position(0, 0),
                           SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(0, move.length);
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_INVALID_DIRECTION,
        scrabble_move_init(&move, "CAT", position(0, 0),
                           (ScrabbleMoveDirection)99));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_INVALID_WORD,
        scrabble_move_init(
            &move, "", position(0, 0), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_INVALID_WORD,
        scrabble_move_init(
            &move, "CA-T", position(0, 0), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_INVALID_WORD,
        scrabble_move_init(
            &move, "ABCDEFGHIJKLMNOP", position(0, 0),
            SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OUT_OF_BOUNDS,
        scrabble_move_init(
            &move, "CAT", position(14, 13), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OUT_OF_BOUNDS,
        scrabble_move_init(
            &move, "CAT", position(13, 14), SCRABBLE_MOVE_VERTICAL));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OUT_OF_BOUNDS,
        scrabble_move_init(
            &move, "CAT", position(15, 0), SCRABBLE_MOVE_HORIZONTAL));

    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "CAT", position(0, 0), SCRABBLE_MOVE_HORIZONTAL));
    memset(&tile, 0xFF, sizeof(tile));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_INVALID_TILE_INDEX,
        scrabble_move_tile_at(&move, 3, &tile));
    TEST_ASSERT_INT('\0', tile.letter);
    TEST_ASSERT_INT(0, tile.from_rack);
    TEST_ASSERT_INT(0, tile.is_blank);
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_INVALID_TILE_INDEX,
        scrabble_move_set_tile_from_rack(&move, 3, 0));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_INVALID_TILE_INDEX,
        scrabble_move_set_tile_blank(&move, 3, 1));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_INVALID_ARGUMENT,
        scrabble_move_tile_at(NULL, 0, &tile));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_INVALID_ARGUMENT,
        scrabble_move_tile_at(&move, 0, NULL));
    TEST_ASSERT_INT(0, scrabble_move_rack_tile_count(NULL));
    TEST_ASSERT(!scrabble_move_covers_position(NULL, position(0, 0)));
    return 0;
}

static const ScrabbleTestCase TESTS[] = {
    {"initializes horizontal move geometry",
     initializes_horizontal_move_geometry},
    {"tracks intersections and blank tiles",
     tracks_intersections_and_blank_tiles},
    {"accepts moves that end at board edges",
     accepts_moves_that_end_at_board_edges},
    {"rejects invalid moves and tile indexes",
     rejects_invalid_moves_and_tile_indexes}
};

TEST_MAIN(TESTS)
