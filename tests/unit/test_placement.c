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

static int validates_and_applies_connected_words(void) {
    ScrabbleDictionary *dictionary = load_dictionary();
    ScrabbleBoard *board = create_cross_word_board();
    ScrabbleRack rack;
    ScrabbleMove move;
    ScrabbleMove validated;
    ScrabbleMove applied;
    ScrabbleBoardCell cell;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(
        SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "TRAIN"));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "TRAIN", position(3, 7), SCRABBLE_MOVE_VERTICAL));

    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_OK,
        scrabble_connected_move_validate(
            board, dictionary, &rack, &move, &validated));
    TEST_ASSERT_INT(5, scrabble_move_rack_tile_count(&validated));
    TEST_ASSERT_INT(3, scrabble_board_tile_count(board));

    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_OK,
        scrabble_connected_move_apply(
            board, dictionary, &rack, &move, &applied));
    TEST_ASSERT_INT(8, scrabble_board_tile_count(board));
    TEST_ASSERT_STRING("TRAIN", applied.word);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_get_cell(board, position(7, 7), &cell));
    TEST_ASSERT_INT('N', cell.letter);
    TEST_ASSERT_INT(0, cell.is_blank);

    scrabble_board_destroy(board);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static int validates_connected_board_rules_without_a_rack(void) {
    ScrabbleDictionary *dictionary = load_dictionary();
    ScrabbleBoard *board = create_cross_word_board();
    ScrabbleRack rack;
    ScrabbleMove move;
    ScrabbleMove validated;
    ScrabbleMoveTile tile;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "TRAIN", position(3, 7), SCRABBLE_MOVE_VERTICAL));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_set_tile_blank(&move, 0, 1));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_OK,
        scrabble_connected_move_validate_board(
            board, dictionary, &move, &validated));
    TEST_ASSERT_INT(5, scrabble_move_rack_tile_count(&validated));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_tile_at(&validated, 0, &tile));
    TEST_ASSERT_INT(1, tile.is_blank);
    TEST_ASSERT_INT(3, scrabble_board_tile_count(board));

    TEST_ASSERT_INT(
        SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "XXXXX"));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_RACK_MISMATCH,
        scrabble_connected_move_validate(
            board, dictionary, &rack, &move, NULL));

    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "TRAIN", position(0, 0), SCRABBLE_MOVE_VERTICAL));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_MOVE_NOT_CONNECTED,
        scrabble_connected_move_validate_board(
            board, dictionary, &move, NULL));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INVALID_ARGUMENT,
        scrabble_connected_move_validate_board(
            NULL, dictionary, &move, NULL));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INVALID_ARGUMENT,
        scrabble_connected_move_validate_board(
            board, NULL, &move, NULL));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INVALID_ARGUMENT,
        scrabble_connected_move_validate_board(
            board, dictionary, NULL, NULL));

    scrabble_board_destroy(board);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static int reuses_board_blanks_and_assigns_new_blanks(void) {
    ScrabbleDictionary *dictionary = load_dictionary();
    ScrabbleBoard *board = scrabble_board_create();
    ScrabbleRack rack;
    ScrabbleMove move;
    ScrabbleMove validated;
    ScrabbleMoveTile tile;
    ScrabbleBoardCell cell;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_place_tile(board, position(7, 7), 'A', 1));
    TEST_ASSERT_INT(
        SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "RI?"));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "RAIN", position(6, 7), SCRABBLE_MOVE_VERTICAL));

    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_OK,
        scrabble_connected_move_validate(
            board, dictionary, &rack, &move, &validated));
    TEST_ASSERT_INT(3, scrabble_move_rack_tile_count(&validated));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_tile_at(&validated, 1, &tile));
    TEST_ASSERT_INT(0, tile.from_rack);
    TEST_ASSERT_INT(0, tile.is_blank);
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_tile_at(&validated, 3, &tile));
    TEST_ASSERT_INT(1, tile.from_rack);
    TEST_ASSERT_INT(1, tile.is_blank);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_get_cell(board, position(7, 7), &cell));
    TEST_ASSERT_INT(1, cell.is_blank);

    scrabble_board_destroy(board);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static int rejects_disconnected_incomplete_and_invalid_cross_words(void) {
    ScrabbleDictionary *dictionary = load_dictionary();
    ScrabbleBoard *board = scrabble_board_create();
    ScrabbleRack rack;
    ScrabbleMove move;
    ScrabbleMove output;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(
        SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "TRAIN"));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "TRAIN", position(0, 0), SCRABBLE_MOVE_VERTICAL));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_BOARD_EMPTY,
        scrabble_connected_move_validate(
            board, dictionary, &rack, &move, NULL));

    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_place_tile(board, position(14, 14), 'A', 0));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_MOVE_NOT_CONNECTED,
        scrabble_connected_move_validate(
            board, dictionary, &rack, &move, NULL));

    scrabble_board_clear(board);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_place_tile(board, position(7, 3), 'T', 0));
    TEST_ASSERT_INT(
        SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "RAIN"));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "RAIN", position(7, 4), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INCOMPLETE_WORD,
        scrabble_connected_move_validate(
            board, dictionary, &rack, &move, NULL));

    scrabble_board_clear(board);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_place_tile(board, position(7, 6), 'X', 0));
    TEST_ASSERT_INT(
        SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "TRAIN"));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "TRAIN", position(3, 7), SCRABBLE_MOVE_VERTICAL));
    memset(&output, 0xFF, sizeof(output));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_CROSS_WORD_NOT_IN_DICTIONARY,
        scrabble_connected_move_apply(
            board, dictionary, &rack, &move, &output));
    TEST_ASSERT_INT(0, output.length);
    TEST_ASSERT_INT(1, scrabble_board_tile_count(board));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INVALID_ARGUMENT,
        scrabble_connected_move_validate(
            NULL, dictionary, &rack, &move, NULL));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INVALID_ARGUMENT,
        scrabble_connected_move_validate(
            board, NULL, &rack, &move, NULL));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INVALID_ARGUMENT,
        scrabble_connected_move_validate(
            board, dictionary, NULL, &move, NULL));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INVALID_ARGUMENT,
        scrabble_connected_move_apply(
            board, dictionary, &rack, NULL, NULL));

    scrabble_board_destroy(board);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static int rejects_connected_rack_mismatches_atomically(void) {
    ScrabbleDictionary *dictionary = load_dictionary();
    ScrabbleBoard *board = create_cross_word_board();
    ScrabbleRack rack;
    ScrabbleMove move;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(
        SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "XXXXX"));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "TRAIN", position(3, 7), SCRABBLE_MOVE_VERTICAL));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_RACK_MISMATCH,
        scrabble_connected_move_apply(
            board, dictionary, &rack, &move, NULL));
    TEST_ASSERT_INT(3, scrabble_board_tile_count(board));

    scrabble_board_destroy(board);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static int resolves_matching_board_letters(void) {
    ScrabbleBoard *board = scrabble_board_create();
    ScrabbleMove move;
    ScrabbleMove resolved;
    ScrabbleMoveTile tile;
    ScrabbleBoardCell cell;

    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_place_tile(board, position(7, 7), 'A', 1));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "cat", position(7, 6), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_set_tile_blank(&move, 0, 1));

    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_OK,
        scrabble_move_resolve_board_tiles(board, &move, &resolved));
    TEST_ASSERT_STRING("CAT", resolved.word);
    TEST_ASSERT_INT(2, scrabble_move_rack_tile_count(&resolved));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_tile_at(&resolved, 0, &tile));
    TEST_ASSERT_INT(1, tile.from_rack);
    TEST_ASSERT_INT(1, tile.is_blank);
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_tile_at(&resolved, 1, &tile));
    TEST_ASSERT_INT('A', tile.letter);
    TEST_ASSERT_INT(0, tile.from_rack);
    TEST_ASSERT_INT(0, tile.is_blank);
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_tile_at(&resolved, 2, &tile));
    TEST_ASSERT_INT(1, tile.from_rack);

    TEST_ASSERT_INT(1, scrabble_board_tile_count(board));
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_get_cell(board, position(7, 7), &cell));
    TEST_ASSERT_INT('A', cell.letter);
    TEST_ASSERT_INT(1, cell.is_blank);

    scrabble_board_destroy(board);
    return 0;
}

static int rejects_conflicting_board_letters_atomically(void) {
    ScrabbleBoard *board = scrabble_board_create();
    ScrabbleMove move;
    ScrabbleMove output;
    ScrabbleBoardCell cell;

    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_place_tile(board, position(7, 7), 'X', 0));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "CAT", position(7, 6), SCRABBLE_MOVE_HORIZONTAL));
    memset(&output, 0xFF, sizeof(output));

    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_LETTER_CONFLICT,
        scrabble_move_resolve_board_tiles(board, &move, &output));
    TEST_ASSERT_INT(0, output.length);
    TEST_ASSERT_INT(1, scrabble_board_tile_count(board));
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_get_cell(board, position(7, 7), &cell));
    TEST_ASSERT_INT('X', cell.letter);

    scrabble_board_destroy(board);
    return 0;
}

static int rejects_moves_that_add_no_tiles(void) {
    ScrabbleBoard *board = scrabble_board_create();
    ScrabbleMove move;
    ScrabbleMove output;

    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_place_tile(board, position(7, 6), 'C', 0));
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_place_tile(board, position(7, 7), 'A', 0));
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_place_tile(board, position(7, 8), 'T', 0));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "CAT", position(7, 6), SCRABBLE_MOVE_HORIZONTAL));
    memset(&output, 0xFF, sizeof(output));

    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_NO_NEW_TILES,
        scrabble_move_resolve_board_tiles(board, &move, &output));
    TEST_ASSERT_INT(0, output.length);
    TEST_ASSERT_INT(3, scrabble_board_tile_count(board));

    scrabble_board_destroy(board);
    return 0;
}

static int leaves_connectivity_for_later_validation(void) {
    ScrabbleBoard *board = scrabble_board_create();
    ScrabbleMove move;
    ScrabbleMove resolved;

    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_place_tile(board, position(7, 7), 'A', 0));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "DOG", position(0, 0), SCRABBLE_MOVE_VERTICAL));

    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_OK,
        scrabble_move_resolve_board_tiles(board, &move, &resolved));
    TEST_ASSERT_INT(3, scrabble_move_rack_tile_count(&resolved));

    move.length = 2;
    memset(&resolved, 0xFF, sizeof(resolved));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INVALID_MOVE,
        scrabble_move_resolve_board_tiles(board, &move, &resolved));
    TEST_ASSERT_INT(0, resolved.length);
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INVALID_ARGUMENT,
        scrabble_move_resolve_board_tiles(NULL, &move, NULL));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INVALID_ARGUMENT,
        scrabble_move_resolve_board_tiles(board, NULL, NULL));

    scrabble_board_destroy(board);
    return 0;
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

static int validates_opening_board_rules_without_a_rack(void) {
    ScrabbleDictionary *dictionary = load_dictionary();
    ScrabbleBoard *board = scrabble_board_create();
    ScrabbleRack rack;
    ScrabbleMove move;
    ScrabbleMove validated;
    ScrabbleMoveTile tile;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "RETAINS", position(7, 4), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_set_tile_blank(&move, 3, 1));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_OK,
        scrabble_opening_move_validate_board(
            board, dictionary, &move, &validated));
    TEST_ASSERT(scrabble_board_is_empty(board));
    TEST_ASSERT_STRING("RETAINS", validated.word);
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_tile_at(&validated, 3, &tile));
    TEST_ASSERT_INT(1, tile.is_blank);

    TEST_ASSERT_INT(
        SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "XXXXXXX"));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_RACK_MISMATCH,
        scrabble_opening_move_validate(
            board, dictionary, &rack, &move, NULL));

    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INVALID_ARGUMENT,
        scrabble_opening_move_validate_board(
            NULL, dictionary, &move, NULL));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INVALID_ARGUMENT,
        scrabble_opening_move_validate_board(
            board, NULL, &move, NULL));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INVALID_ARGUMENT,
        scrabble_opening_move_validate_board(
            board, dictionary, NULL, NULL));

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
    {"validates and applies connected words",
     validates_and_applies_connected_words},
    {"validates connected board rules without a rack",
     validates_connected_board_rules_without_a_rack},
    {"reuses board blanks and assigns new blanks",
     reuses_board_blanks_and_assigns_new_blanks},
    {"rejects disconnected, incomplete, and invalid cross words",
     rejects_disconnected_incomplete_and_invalid_cross_words},
    {"rejects connected rack mismatches atomically",
     rejects_connected_rack_mismatches_atomically},
    {"resolves matching board letters",
     resolves_matching_board_letters},
    {"rejects conflicting board letters atomically",
     rejects_conflicting_board_letters_atomically},
    {"rejects moves that add no tiles",
     rejects_moves_that_add_no_tiles},
    {"leaves connectivity for later validation",
     leaves_connectivity_for_later_validation},
    {"validates without changing the board",
     validates_without_changing_the_board},
    {"validates opening board rules without a rack",
     validates_opening_board_rules_without_a_rack},
    {"applies a centered horizontal word",
     applies_a_centered_horizontal_word},
    {"assigns and preserves blank tiles",
     assigns_and_preserves_blank_tiles},
    {"rejects invalid opening moves atomically",
     rejects_invalid_opening_moves_atomically}
};

TEST_MAIN(TESTS)
