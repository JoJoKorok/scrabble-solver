#include "scrabble/rack.h"
#include "scrabble/scoring.h"

#include "test.h"

static int scores_standard_words(void) {
    TEST_ASSERT_INT(22, scrabble_score_word("QUIZ"));
    TEST_ASSERT_INT(14, scrabble_score_word("Scrabble"));
    TEST_ASSERT_INT(0, scrabble_score_word(""));
    return 0;
}

static int rejects_invalid_words(void) {
    TEST_ASSERT_INT(-1, scrabble_score_word(NULL));
    TEST_ASSERT_INT(-1, scrabble_score_word("TWO-WORDS"));
    TEST_ASSERT_INT(-1, scrabble_score_word("WORD2"));
    return 0;
}

static int scores_blank_tiles_as_zero(void) {
    ScrabbleRack rack;
    ScrabbleRackMatch match;

    TEST_ASSERT_INT(SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "QUI?"));
    TEST_ASSERT(scrabble_rack_match(&rack, "QUIZ", &match));
    TEST_ASSERT_INT(1, match.blanks_used);
    TEST_ASSERT_INT(12, scrabble_score_rack_match("QUIZ", &match));
    TEST_ASSERT_INT(12, scrabble_score_rack_match("quiz", &match));
    return 0;
}

static int rejects_inconsistent_blank_matches(void) {
    ScrabbleRackMatch match = {{0}, 1};

    TEST_ASSERT_INT(-1, scrabble_score_rack_match("QUIZ", &match));
    match.blank_letter_counts['Z' - 'A'] = 1;
    match.blanks_used = 0;
    TEST_ASSERT_INT(-1, scrabble_score_rack_match("QUIZ", &match));
    TEST_ASSERT_INT(-1, scrabble_score_rack_match(NULL, &match));
    TEST_ASSERT_INT(-1, scrabble_score_rack_match("QUIZ", NULL));
    return 0;
}

static ScrabbleBoardPosition position(size_t row, size_t column) {
    ScrabbleBoardPosition value = {row, column};

    return value;
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

static int scores_opening_premiums_and_bingos(void) {
    ScrabbleMove move;
    ScrabbleMoveScore score;

    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "RETAINS", position(7, 4), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_SCORING_OK, scrabble_score_opening_move(&move, &score));
    TEST_ASSERT_INT(7, score.letter_score);
    TEST_ASSERT_INT(2, score.word_multiplier);
    TEST_ASSERT_INT(50, score.bingo_bonus);
    TEST_ASSERT_INT(64, score.total_score);

    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "QUIZZES", position(7, 3), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_SCORING_OK, scrabble_score_opening_move(&move, &score));
    TEST_ASSERT_INT(44, score.letter_score);
    TEST_ASSERT_INT(2, score.word_multiplier);
    TEST_ASSERT_INT(50, score.bingo_bonus);
    TEST_ASSERT_INT(138, score.total_score);
    return 0;
}

static int scores_opening_blank_tiles_as_zero(void) {
    ScrabbleMove move;
    ScrabbleMoveScore score;

    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "RETAINS", position(4, 7), SCRABBLE_MOVE_VERTICAL));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_set_tile_blank(&move, 3, 1));
    TEST_ASSERT_INT(
        SCRABBLE_SCORING_OK, scrabble_score_opening_move(&move, &score));
    TEST_ASSERT_INT(6, score.letter_score);
    TEST_ASSERT_INT(2, score.word_multiplier);
    TEST_ASSERT_INT(50, score.bingo_bonus);
    TEST_ASSERT_INT(62, score.total_score);
    return 0;
}

static int rejects_invalid_opening_scores(void) {
    ScrabbleMove move;
    ScrabbleMoveScore score = {
        .letter_score = 1,
        .word_multiplier = 1,
        .cross_word_score = 1,
        .bingo_bonus = 1,
        .total_score = 1
    };

    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "RETAINS", position(0, 0), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_SCORING_INVALID_MOVE,
        scrabble_score_opening_move(&move, &score));
    TEST_ASSERT_INT(0, score.total_score);

    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "A", position(7, 7), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_SCORING_INVALID_MOVE,
        scrabble_score_opening_move(&move, &score));
    TEST_ASSERT_INT(
        SCRABBLE_SCORING_INVALID_ARGUMENT,
        scrabble_score_opening_move(NULL, &score));
    TEST_ASSERT_INT(
        SCRABBLE_SCORING_INVALID_ARGUMENT,
        scrabble_score_opening_move(&move, NULL));
    return 0;
}

static int scores_main_and_cross_words(void) {
    ScrabbleBoard *board = create_cross_word_board();
    ScrabbleMove move;
    ScrabbleMoveScore score;

    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "TRAIN", position(3, 7), SCRABBLE_MOVE_VERTICAL));
    TEST_ASSERT_INT(
        SCRABBLE_SCORING_OK,
        scrabble_score_move(board, &move, &score));
    TEST_ASSERT_INT(6, score.letter_score);
    TEST_ASSERT_INT(2, score.word_multiplier);
    TEST_ASSERT_INT(8, score.cross_word_score);
    TEST_ASSERT_INT(0, score.bingo_bonus);
    TEST_ASSERT_INT(20, score.total_score);
    TEST_ASSERT_INT(3, scrabble_board_tile_count(board));

    scrabble_board_destroy(board);
    return 0;
}

static int scores_new_and_existing_blanks_as_zero(void) {
    ScrabbleBoard *board = create_cross_word_board();
    ScrabbleMove move;
    ScrabbleMoveScore score;

    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "TRAIN", position(3, 7), SCRABBLE_MOVE_VERTICAL));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK, scrabble_move_set_tile_blank(&move, 4, 1));
    TEST_ASSERT_INT(
        SCRABBLE_SCORING_OK,
        scrabble_score_move(board, &move, &score));
    TEST_ASSERT_INT(5, score.letter_score);
    TEST_ASSERT_INT(2, score.word_multiplier);
    TEST_ASSERT_INT(6, score.cross_word_score);
    TEST_ASSERT_INT(16, score.total_score);

    scrabble_board_clear(board);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_place_tile(board, position(7, 7), 'A', 1));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "RAIN", position(6, 7), SCRABBLE_MOVE_VERTICAL));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_set_tile_from_rack(&move, 1, 0));
    TEST_ASSERT_INT(
        SCRABBLE_SCORING_OK,
        scrabble_score_move(board, &move, &score));
    TEST_ASSERT_INT(3, score.letter_score);
    TEST_ASSERT_INT(1, score.word_multiplier);
    TEST_ASSERT_INT(0, score.cross_word_score);
    TEST_ASSERT_INT(3, score.total_score);

    scrabble_board_destroy(board);
    return 0;
}

static int rejects_invalid_connected_scores(void) {
    ScrabbleBoard *board = scrabble_board_create();
    ScrabbleMove move;
    ScrabbleMoveScore score = {
        .letter_score = 1,
        .word_multiplier = 1,
        .cross_word_score = 1,
        .bingo_bonus = 1,
        .total_score = 1
    };

    TEST_ASSERT(board != NULL);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_place_tile(board, position(14, 14), 'A', 0));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "DOG", position(0, 0), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_SCORING_INVALID_MOVE,
        scrabble_score_move(board, &move, &score));
    TEST_ASSERT_INT(0, score.total_score);

    scrabble_board_clear(board);
    TEST_ASSERT_INT(
        SCRABBLE_BOARD_OK,
        scrabble_board_place_tile(board, position(7, 3), 'T', 0));
    TEST_ASSERT_INT(
        SCRABBLE_MOVE_OK,
        scrabble_move_init(
            &move, "RAIN", position(7, 4), SCRABBLE_MOVE_HORIZONTAL));
    TEST_ASSERT_INT(
        SCRABBLE_SCORING_INVALID_MOVE,
        scrabble_score_move(board, &move, &score));
    TEST_ASSERT_INT(
        SCRABBLE_SCORING_INVALID_ARGUMENT,
        scrabble_score_move(NULL, &move, &score));
    TEST_ASSERT_INT(
        SCRABBLE_SCORING_INVALID_ARGUMENT,
        scrabble_score_move(board, NULL, &score));
    TEST_ASSERT_INT(
        SCRABBLE_SCORING_INVALID_ARGUMENT,
        scrabble_score_move(board, &move, NULL));

    scrabble_board_destroy(board);
    return 0;
}

static const ScrabbleTestCase TESTS[] = {
    {"scores standard words", scores_standard_words},
    {"rejects invalid words", rejects_invalid_words},
    {"scores blank tiles as zero", scores_blank_tiles_as_zero},
    {"rejects inconsistent blank matches", rejects_inconsistent_blank_matches},
    {"scores opening premiums and bingos",
     scores_opening_premiums_and_bingos},
    {"scores opening blank tiles as zero",
     scores_opening_blank_tiles_as_zero},
    {"rejects invalid opening scores", rejects_invalid_opening_scores},
    {"scores main and cross words", scores_main_and_cross_words},
    {"scores new and existing blanks as zero",
     scores_new_and_existing_blanks_as_zero},
    {"rejects invalid connected scores",
     rejects_invalid_connected_scores}
};

TEST_MAIN(TESTS)
