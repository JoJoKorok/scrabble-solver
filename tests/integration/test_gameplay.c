#include "scrabble/game.h"
#include "scrabble/solver.h"

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

static const ScrabbleResult *find_word(
    const ScrabbleResultSet *results,
    const char *word) {
    for (size_t index = 0; index < results->count; ++index) {
        if (strcmp(results->items[index].word, word) == 0) {
            return &results->items[index];
        }
    }
    return NULL;
}

static const ScrabbleResult *find_placement(
    const ScrabbleResultSet *results,
    const char *word,
    ScrabbleBoardPosition start,
    ScrabbleMoveDirection direction) {
    for (size_t index = 0; index < results->count; ++index) {
        const ScrabbleResult *result = &results->items[index];

        if (result->has_placement &&
            strcmp(result->word, word) == 0 &&
            result->move.start.row == start.row &&
            result->move.start.column == start.column &&
            result->move.direction == direction) {
            return result;
        }
    }
    return NULL;
}

static int racks_equal(
    const ScrabbleRack *left,
    const ScrabbleRack *right) {
    return left->blank_count == right->blank_count &&
        left->tile_count == right->tile_count &&
        memcmp(
            left->letter_counts,
            right->letter_counts,
            sizeof(left->letter_counts)) == 0;
}

static int apply_board_suggestion(
    ScrabbleGame *game,
    const ScrabbleDictionary *dictionary,
    const char *rack_text,
    const char *word,
    ScrabbleBoardPosition start,
    ScrabbleMoveDirection direction) {
    ScrabbleRack rack;
    ScrabbleResultSet results = {0};
    const ScrabbleResult *result;
    const ScrabbleGameTurn *turn;
    ScrabblePlacementStatus placement_status;
    size_t turn_index = scrabble_game_move_count(game);
    int suggested_score;

    TEST_ASSERT_INT(SCRABBLE_RACK_OK,
                    scrabble_rack_init(&rack, rack_text));
    TEST_ASSERT_INT(
        SCRABBLE_SOLVER_OK,
        scrabble_solve_board(
            dictionary, &rack, scrabble_game_board(game), &results));
    result = find_placement(&results, word, start, direction);
    TEST_ASSERT(result != NULL);
    suggested_score = result->score;
    TEST_ASSERT_INT(
        SCRABBLE_GAME_OK,
        scrabble_game_apply_move(
            game,
            dictionary,
            &rack,
            &result->move,
            NULL,
            &placement_status));
    TEST_ASSERT_INT(SCRABBLE_PLACEMENT_OK, placement_status);
    turn = scrabble_game_turn_at(game, turn_index);
    TEST_ASSERT(turn != NULL);
    TEST_ASSERT_STRING(word, turn->move.word);
    TEST_ASSERT_INT(SCRABBLE_TURN_OWNER_USER, turn->owner);
    TEST_ASSERT_INT(suggested_score, turn->score.total_score);
    scrabble_result_set_destroy(&results);
    return 0;
}

static int completes_an_alternating_two_player_workflow(void) {
    ScrabbleDictionary *dictionary = load_dictionary();
    ScrabbleGame *game = scrabble_game_create();
    ScrabbleRack rack;
    ScrabbleRack rack_before_opponent_move;
    ScrabbleMove move;
    ScrabbleMove undone;
    ScrabbleBoardCell cell;
    const ScrabbleGameTurn *turn;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT(game != NULL);
    TEST_ASSERT_INT(SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "RIN"));

    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move,
            "RETAINS",
            position(7, 4),
            SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_set_tile_blank(&move, 3, 1));
    rack_before_opponent_move = rack;
    TEST_ASSERT_INT(
        SCRABBLE_GAME_OK,
        scrabble_game_record_opponent_move(
            game, dictionary, &move, NULL, NULL));
    TEST_ASSERT(racks_equal(&rack, &rack_before_opponent_move));
    turn = scrabble_game_turn_at(game, 0);
    TEST_ASSERT(turn != NULL);
    TEST_ASSERT_INT(SCRABBLE_TURN_OWNER_OPPONENT, turn->owner);
    TEST_ASSERT_INT(62, turn->score.total_score);

    TEST_ASSERT_INT(
        0,
        apply_board_suggestion(
            game,
            dictionary,
            "RIN",
            "RAIN",
            position(6, 7),
            SCRABBLE_MOVE_VERTICAL));

    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move,
            "STAIN",
            position(7, 10),
            SCRABBLE_MOVE_VERTICAL));
    rack_before_opponent_move = rack;
    TEST_ASSERT_INT(
        SCRABBLE_GAME_OK,
        scrabble_game_record_opponent_move(
            game, dictionary, &move, NULL, NULL));
    TEST_ASSERT(racks_equal(&rack, &rack_before_opponent_move));
    turn = scrabble_game_turn_at(game, 2);
    TEST_ASSERT(turn != NULL);
    TEST_ASSERT_STRING("STAIN", turn->move.word);
    TEST_ASSERT_INT(SCRABBLE_TURN_OWNER_OPPONENT, turn->owner);

    TEST_ASSERT_INT(
        0,
        apply_board_suggestion(
            game,
            dictionary,
            "TRAI",
            "TRAIN",
            position(11, 6),
            SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(4, scrabble_game_move_count(game));
    TEST_ASSERT_INT(18,
                    scrabble_board_tile_count(scrabble_game_board(game)));
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_get_cell(
            scrabble_game_board(game), position(7, 7), &cell));
    TEST_ASSERT_INT('A', cell.letter);
    TEST_ASSERT_INT(1, cell.is_blank);

    TEST_ASSERT_INT(
        SCRABBLE_GAME_OK, scrabble_game_undo_last_move(game, &undone));
    TEST_ASSERT_STRING("TRAIN", undone.word);
    TEST_ASSERT_INT(14,
                    scrabble_board_tile_count(scrabble_game_board(game)));
    turn = scrabble_game_turn_at(game, 2);
    TEST_ASSERT(turn != NULL);
    TEST_ASSERT_INT(SCRABBLE_TURN_OWNER_OPPONENT, turn->owner);

    scrabble_game_reset(game);
    TEST_ASSERT(scrabble_board_is_empty(scrabble_game_board(game)));
    TEST_ASSERT_INT(0, scrabble_game_move_count(game));

    scrabble_game_destroy(game);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static int completes_a_multi_turn_solver_workflow(void) {
    ScrabbleDictionary *dictionary = load_dictionary();
    ScrabbleGame *game = scrabble_game_create();
    ScrabbleRack rack;
    ScrabbleResultSet results = {0};
    const ScrabbleResult *opening;
    const ScrabbleGameTurn *turn;
    ScrabbleMove move;
    ScrabbleMove undone;
    ScrabbleBoardCell cell;

    TEST_ASSERT(dictionary != NULL);
    TEST_ASSERT(game != NULL);

    TEST_ASSERT_INT(
        SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "RETAINS"));
    TEST_ASSERT_INT(
        SCRABBLE_SOLVER_OK, scrabble_solve(dictionary, &rack, &results));
    opening = find_word(&results, "RETAINS");
    TEST_ASSERT(opening != NULL);
    TEST_ASSERT_INT(7, opening->score);
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move,
            opening->word,
            position(7, 4),
            SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_GAME_OK,
        scrabble_game_apply_opening_move(
            game, dictionary, &rack, &move, NULL, NULL));
    scrabble_result_set_destroy(&results);
    turn = scrabble_game_turn_at(game, 0);
    TEST_ASSERT(turn != NULL);
    TEST_ASSERT_INT(64, turn->score.total_score);

    TEST_ASSERT_INT(
        0,
        apply_board_suggestion(
            game,
            dictionary,
            "TAIN",
            "STAIN",
            position(7, 10),
            SCRABBLE_MOVE_VERTICAL));
    TEST_ASSERT_INT(
        0,
        apply_board_suggestion(
            game,
            dictionary,
            "TRAI",
            "TRAIN",
            position(11, 6),
            SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        0,
        apply_board_suggestion(
            game,
            dictionary,
            "AIN",
            "RAIN",
            position(11, 7),
            SCRABBLE_MOVE_VERTICAL));
    TEST_ASSERT_INT(4, scrabble_game_move_count(game));
    TEST_ASSERT_INT(18,
                    scrabble_board_tile_count(scrabble_game_board(game)));

    TEST_ASSERT_INT(
        SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "XXXXXXX"));
    TEST_ASSERT_INT(
        SCRABBLE_SOLVER_OK,
        scrabble_solve_board(
            dictionary, &rack, scrabble_game_board(game), &results));
    TEST_ASSERT_INT(0, results.count);
    scrabble_result_set_destroy(&results);
    TEST_ASSERT_INT(18,
                    scrabble_board_tile_count(scrabble_game_board(game)));

    TEST_ASSERT_INT(
        SCRABBLE_GAME_OK, scrabble_game_undo_last_move(game, &undone));
    TEST_ASSERT_STRING("RAIN", undone.word);
    TEST_ASSERT_INT(15,
                    scrabble_board_tile_count(scrabble_game_board(game)));
    TEST_ASSERT_INT(
        SCRABBLE_GAME_OK, scrabble_game_undo_last_move(game, &undone));
    TEST_ASSERT_STRING("TRAIN", undone.word);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_get_cell(
            scrabble_game_board(game), position(11, 10), &cell));
    TEST_ASSERT_INT('N', cell.letter);
    TEST_ASSERT_INT(11,
                    scrabble_board_tile_count(scrabble_game_board(game)));
    TEST_ASSERT_INT(
        SCRABBLE_GAME_OK, scrabble_game_undo_last_move(game, &undone));
    TEST_ASSERT_STRING("STAIN", undone.word);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_get_cell(
            scrabble_game_board(game), position(7, 10), &cell));
    TEST_ASSERT_INT('S', cell.letter);
    TEST_ASSERT_INT(7,
                    scrabble_board_tile_count(scrabble_game_board(game)));

    scrabble_game_reset(game);
    TEST_ASSERT(scrabble_board_is_empty(scrabble_game_board(game)));
    TEST_ASSERT_INT(0, scrabble_game_move_count(game));
    TEST_ASSERT_INT(
        SCRABBLE_GAME_NO_MOVES,
        scrabble_game_undo_last_move(game, NULL));

    scrabble_game_destroy(game);
    scrabble_dictionary_destroy(dictionary);
    return 0;
}

static const ScrabbleTestCase TESTS[] = {
    {"completes a multi-turn solver workflow",
     completes_a_multi_turn_solver_workflow},
    {"completes an alternating two-player workflow",
     completes_an_alternating_two_player_workflow}
};

TEST_MAIN(TESTS)
