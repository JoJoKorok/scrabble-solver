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
    ScrabbleMoveScore score = {1, 1, 1, 1};

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

static const ScrabbleTestCase TESTS[] = {
    {"scores standard words", scores_standard_words},
    {"rejects invalid words", rejects_invalid_words},
    {"scores blank tiles as zero", scores_blank_tiles_as_zero},
    {"rejects inconsistent blank matches", rejects_inconsistent_blank_matches},
    {"scores opening premiums and bingos",
     scores_opening_premiums_and_bingos},
    {"scores opening blank tiles as zero",
     scores_opening_blank_tiles_as_zero},
    {"rejects invalid opening scores", rejects_invalid_opening_scores}
};

TEST_MAIN(TESTS)
