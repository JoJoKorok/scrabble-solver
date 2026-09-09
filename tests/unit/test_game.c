#include "scrabble/game.h"

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

static ScrabbleDictionary *load_opponent_dictionary(void) {
    ScrabbleDictionaryStatus status;
    ScrabbleDictionary *dictionary = scrabble_dictionary_load(
        SCRABBLE_OPPONENT_DICTIONARY_FIXTURE, &status);

    return status == SCRABBLE_DICTIONARY_OK ? dictionary : NULL;
}

static int creates_and_resets_game_state(void) {
    ScrabbleGame *game = scrabble_game_create();

    TEST_ASSERT(game != NULL);
    TEST_ASSERT(scrabble_game_board(game) != NULL);
    TEST_ASSERT(scrabble_board_is_empty(scrabble_game_board(game)));
    TEST_ASSERT_INT(0, scrabble_game_move_count(game));
    TEST_ASSERT(scrabble_game_move_at(game, 0) == NULL);
    scrabble_game_reset(game);
    TEST_ASSERT(scrabble_board_is_empty(scrabble_game_board(game)));
    TEST_ASSERT_INT(0, scrabble_game_move_count(game));

    TEST_ASSERT(scrabble_game_board(NULL) == NULL);
    TEST_ASSERT_INT(0, scrabble_game_move_count(NULL));
    TEST_ASSERT(scrabble_game_move_at(NULL, 0) == NULL);
    TEST_ASSERT(scrabble_game_turn_at(NULL, 0) == NULL);
    scrabble_game_reset(NULL);
    scrabble_game_destroy(NULL);
    scrabble_game_destroy(game);
    return 0;
}

static int records_and_undoes_an_opening_move(void) {
    ScrabbleDictionary *dictionary = load_dictionary();
    ScrabbleGame *game = scrabble_game_create();
    ScrabbleRack rack;
    ScrabbleMove move;
    ScrabbleMove applied;
    ScrabbleMove undone;
    ScrabblePlacementStatus placement_status;
    const ScrabbleMove *recorded;
    const ScrabbleGameTurn *turn;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT(game != NULL);
    TEST_ASSERT_INT(SCRABBLE_RACK_OK,
                    scrabble_rack_init(&rack, "RET?INS"));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "RETAINS", position(7, 4), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_GAME_OK,
        scrabble_game_apply_opening_move(
            game,
            dictionary,
            &rack,
            &move,
            &applied,
            &placement_status));
    TEST_ASSERT_INT(SCRABBLE_PLACEMENT_OK, placement_status);
    TEST_ASSERT_INT(1, scrabble_game_move_count(game));
    TEST_ASSERT_INT(7,
                    scrabble_board_tile_count(scrabble_game_board(game)));
    recorded = scrabble_game_move_at(game, 0);
    TEST_ASSERT(recorded != NULL);
    TEST_ASSERT_STRING("RETAINS", recorded->word);
    TEST_ASSERT_INT(applied.blank_tile_mask, recorded->blank_tile_mask);
    turn = scrabble_game_turn_at(game, 0);
    TEST_ASSERT(turn != NULL);
    TEST_ASSERT_STRING("RETAINS", turn->move.word);
    TEST_ASSERT_INT(62, turn->score.total_score);
    TEST_ASSERT_INT(SCRABBLE_TURN_OWNER_USER, turn->owner);
    TEST_ASSERT(scrabble_game_turn_at(game, 1) == NULL);

    TEST_ASSERT_INT(
        SCRABBLE_GAME_OK,
        scrabble_game_undo_last_move(game, &undone));
    TEST_ASSERT_STRING("RETAINS", undone.word);
    TEST_ASSERT_INT(0, scrabble_game_move_count(game));
    TEST_ASSERT(scrabble_board_is_empty(scrabble_game_board(game)));

    memset(&undone, 0xFF, sizeof(undone));
    TEST_ASSERT_INT(
        SCRABBLE_GAME_NO_MOVES,
        scrabble_game_undo_last_move(game, &undone));
    TEST_ASSERT_INT(0, undone.length);

    scrabble_game_destroy(game);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static int resets_a_recorded_game_for_reuse(void) {
    ScrabbleDictionary *dictionary = load_dictionary();
    ScrabbleGame *game = scrabble_game_create();
    ScrabbleRack rack;
    ScrabbleMove move;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT(game != NULL);
    TEST_ASSERT_INT(SCRABBLE_RACK_OK,
                    scrabble_rack_init(&rack, "RETAINS"));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "RETAINS", position(4, 7), SCRABBLE_MOVE_VERTICAL));
    TEST_ASSERT_INT(
        SCRABBLE_GAME_OK,
        scrabble_game_apply_opening_move(
            game, dictionary, &rack, &move, NULL, NULL));
    scrabble_game_reset(game);
    TEST_ASSERT(scrabble_board_is_empty(scrabble_game_board(game)));
    TEST_ASSERT_INT(0, scrabble_game_move_count(game));

    TEST_ASSERT_INT(
        SCRABBLE_GAME_OK,
        scrabble_game_apply_opening_move(
            game, dictionary, &rack, &move, NULL, NULL));
    TEST_ASSERT_INT(1, scrabble_game_move_count(game));

    scrabble_game_destroy(game);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static int records_scores_and_undoes_connected_moves(void) {
    ScrabbleDictionary *dictionary = load_dictionary();
    ScrabbleGame *game = scrabble_game_create();
    ScrabbleRack rack;
    ScrabbleMove move;
    ScrabbleMove applied;
    ScrabbleMove undone;
    ScrabbleMoveTile tile;
    ScrabblePlacementStatus placement_status;
    const ScrabbleGameTurn *turn;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT(game != NULL);
    TEST_ASSERT_INT(
        SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "RETAINS"));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "RETAINS", position(7, 4), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_GAME_OK,
        scrabble_game_apply_opening_move(
            game, dictionary, &rack, &move, NULL, NULL));

    TEST_ASSERT_INT(
        SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "RIN"));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "RAIN", position(6, 7), SCRABBLE_MOVE_VERTICAL));
    TEST_ASSERT_INT(
        SCRABBLE_GAME_OK,
        scrabble_game_apply_move(
            game,
            dictionary,
            &rack,
            &move,
            &applied,
            &placement_status));
    TEST_ASSERT_INT(SCRABBLE_PLACEMENT_OK, placement_status);
    TEST_ASSERT_INT(2, scrabble_game_move_count(game));
    TEST_ASSERT_INT(
        10, scrabble_board_tile_count(scrabble_game_board(game)));
    TEST_ASSERT_INT(3, scrabble_move_rack_tile_count(&applied));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_tile_at(&applied, 1, &tile));
    TEST_ASSERT_INT(0, tile.from_rack);

    turn = scrabble_game_turn_at(game, 1);
    TEST_ASSERT(turn != NULL);
    TEST_ASSERT_STRING("RAIN", turn->move.word);
    TEST_ASSERT_INT(4, turn->score.total_score);
    TEST_ASSERT_INT(0, turn->score.cross_word_score);
    TEST_ASSERT_INT(SCRABBLE_TURN_OWNER_USER, turn->owner);

    TEST_ASSERT_INT(
        SCRABBLE_GAME_OK,
        scrabble_game_undo_last_move(game, &undone));
    TEST_ASSERT_STRING("RAIN", undone.word);
    TEST_ASSERT_INT(1, scrabble_game_move_count(game));
    TEST_ASSERT_INT(
        7, scrabble_board_tile_count(scrabble_game_board(game)));

    TEST_ASSERT_INT(
        SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "TRAIN"));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "TRAIN", position(0, 0), SCRABBLE_MOVE_VERTICAL));
    memset(&applied, 0xFF, sizeof(applied));
    TEST_ASSERT_INT(
        SCRABBLE_GAME_PLACEMENT_REJECTED,
        scrabble_game_apply_move(
            game,
            dictionary,
            &rack,
            &move,
            &applied,
            &placement_status));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_MOVE_NOT_CONNECTED, placement_status);
    TEST_ASSERT_INT(0, applied.length);
    TEST_ASSERT_INT(1, scrabble_game_move_count(game));
    TEST_ASSERT_INT(
        7, scrabble_board_tile_count(scrabble_game_board(game)));

    scrabble_game_destroy(game);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static int records_opponent_moves_without_their_hidden_rack(void) {
    ScrabbleDictionary *dictionary = load_opponent_dictionary();
    ScrabbleGame *game = scrabble_game_create();
    ScrabbleMove move;
    ScrabbleMove applied;
    ScrabbleMove undone;
    ScrabblePlacementStatus placement_status;
    const ScrabbleGameTurn *turn;
    ScrabbleBoardCell cell;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT(game != NULL);
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "VISE", position(7, 7), SCRABBLE_MOVE_VERTICAL));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_set_tile_blank(&move, 1, 1));
    TEST_ASSERT_INT(
        SCRABBLE_GAME_OK,
        scrabble_game_record_opponent_move(
            game,
            dictionary,
            &move,
            &applied,
            &placement_status));
    TEST_ASSERT_INT(SCRABBLE_PLACEMENT_OK, placement_status);
    TEST_ASSERT_STRING("VISE", applied.word);
    TEST_ASSERT_INT(1, scrabble_game_move_count(game));
    TEST_ASSERT_INT(4,
                    scrabble_board_tile_count(scrabble_game_board(game)));
    turn = scrabble_game_turn_at(game, 0);
    TEST_ASSERT(turn != NULL);
    TEST_ASSERT_INT(SCRABBLE_TURN_OWNER_OPPONENT, turn->owner);
    TEST_ASSERT_INT(12, turn->score.total_score);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_get_cell(
            scrabble_game_board(game), position(8, 7), &cell));
    TEST_ASSERT_INT('I', cell.letter);
    TEST_ASSERT_INT(1, cell.is_blank);

    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "RAIN", position(8, 5), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_GAME_OK,
        scrabble_game_record_opponent_move(
            game, dictionary, &move, NULL, &placement_status));
    TEST_ASSERT_INT(SCRABBLE_PLACEMENT_OK, placement_status);
    TEST_ASSERT_INT(2, scrabble_game_move_count(game));
    TEST_ASSERT_INT(7,
                    scrabble_board_tile_count(scrabble_game_board(game)));
    turn = scrabble_game_turn_at(game, 1);
    TEST_ASSERT(turn != NULL);
    TEST_ASSERT_STRING("RAIN", turn->move.word);
    TEST_ASSERT_INT(SCRABBLE_TURN_OWNER_OPPONENT, turn->owner);
    TEST_ASSERT_INT(5, turn->score.total_score);

    TEST_ASSERT_INT(
        SCRABBLE_GAME_OK,
        scrabble_game_undo_last_move(game, &undone));
    TEST_ASSERT_STRING("RAIN", undone.word);
    TEST_ASSERT_INT(1, scrabble_game_move_count(game));
    TEST_ASSERT_INT(4,
                    scrabble_board_tile_count(scrabble_game_board(game)));
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_get_cell(
            scrabble_game_board(game), position(8, 7), &cell));
    TEST_ASSERT_INT('I', cell.letter);
    TEST_ASSERT_INT(1, cell.is_blank);

    scrabble_game_destroy(game);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static int rejected_moves_do_not_change_history(void) {
    ScrabbleDictionary *dictionary = load_dictionary();
    ScrabbleGame *game = scrabble_game_create();
    ScrabbleRack rack;
    ScrabbleMove move;
    ScrabbleMove applied;
    ScrabblePlacementStatus placement_status;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT(game != NULL);
    TEST_ASSERT_INT(SCRABBLE_RACK_OK,
                    scrabble_rack_init(&rack, "RETAINS"));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "RETAINS", position(0, 0), SCRABBLE_MOVE_HORIZONTAL));
    memset(&applied, 0xFF, sizeof(applied));
    TEST_ASSERT_INT(
        SCRABBLE_GAME_PLACEMENT_REJECTED,
        scrabble_game_apply_opening_move(
            game,
            dictionary,
            &rack,
            &move,
            &applied,
            &placement_status));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_MOVE_NOT_CENTERED, placement_status);
    TEST_ASSERT_INT(0, applied.length);
    TEST_ASSERT_INT(0, scrabble_game_move_count(game));
    TEST_ASSERT(scrabble_board_is_empty(scrabble_game_board(game)));

    TEST_ASSERT_INT(
        SCRABBLE_GAME_INVALID_ARGUMENT,
        scrabble_game_apply_opening_move(
            NULL,
            dictionary,
            &rack,
            &move,
            NULL,
            &placement_status));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INVALID_ARGUMENT, placement_status);
    TEST_ASSERT_INT(
        SCRABBLE_GAME_INVALID_ARGUMENT,
        scrabble_game_undo_last_move(NULL, NULL));
    TEST_ASSERT_INT(
        SCRABBLE_GAME_INVALID_ARGUMENT,
        scrabble_game_record_opponent_move(
            NULL, dictionary, &move, NULL, &placement_status));
    TEST_ASSERT_INT(
        SCRABBLE_PLACEMENT_INVALID_ARGUMENT, placement_status);

    scrabble_game_destroy(game);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static const ScrabbleTestCase TESTS[] = {
    {"creates and resets game state", creates_and_resets_game_state},
    {"records and undoes an opening move",
     records_and_undoes_an_opening_move},
    {"resets a recorded game for reuse",
     resets_a_recorded_game_for_reuse},
    {"records, scores, and undoes connected moves",
     records_scores_and_undoes_connected_moves},
    {"records opponent moves without their hidden rack",
     records_opponent_moves_without_their_hidden_rack},
    {"rejected moves do not change history",
     rejected_moves_do_not_change_history}
};

TEST_MAIN(TESTS)
