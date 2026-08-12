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

static const ScrabbleTestCase TESTS[] = {
    {"scores standard words", scores_standard_words},
    {"rejects invalid words", rejects_invalid_words},
    {"scores blank tiles as zero", scores_blank_tiles_as_zero},
    {"rejects inconsistent blank matches", rejects_inconsistent_blank_matches}
};

TEST_MAIN(TESTS)
