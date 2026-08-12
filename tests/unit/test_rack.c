#include "scrabble/rack.h"

#include "test.h"

#include <string.h>

static int initializes_letters_and_blanks(void) {
    ScrabbleRack rack;

    TEST_ASSERT_INT(SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "aAb?*"));
    TEST_ASSERT_INT(5, rack.tile_count);
    TEST_ASSERT_INT(2, rack.letter_counts['A' - 'A']);
    TEST_ASSERT_INT(1, rack.letter_counts['B' - 'A']);
    TEST_ASSERT_INT(2, rack.blank_count);
    return 0;
}

static int validates_rack_input(void) {
    ScrabbleRack rack;

    TEST_ASSERT_INT(SCRABBLE_RACK_TOO_MANY_TILES,
                    scrabble_rack_init(&rack, "ABCDEFGH"));
    TEST_ASSERT_INT(0, rack.tile_count);
    TEST_ASSERT_INT(SCRABBLE_RACK_INVALID_CHARACTER,
                    scrabble_rack_init(&rack, "ABC-DEF"));
    TEST_ASSERT_INT(0, rack.tile_count);
    TEST_ASSERT_INT(SCRABBLE_RACK_INVALID_ARGUMENT,
                    scrabble_rack_init(NULL, "ABC"));
    TEST_ASSERT_INT(SCRABBLE_RACK_INVALID_ARGUMENT,
                    scrabble_rack_init(&rack, NULL));
    return 0;
}

static int matches_repeated_letters(void) {
    ScrabbleRack rack;

    TEST_ASSERT_INT(SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "LETTERS"));
    TEST_ASSERT(scrabble_rack_can_form(&rack, "LETTER"));
    TEST_ASSERT(scrabble_rack_can_form(&rack, "tree"));
    TEST_ASSERT(!scrabble_rack_can_form(&rack, "TELLER"));
    TEST_ASSERT(!scrabble_rack_can_form(&rack, ""));
    return 0;
}

static int records_blank_substitutions(void) {
    ScrabbleRack rack;
    ScrabbleRackMatch match;

    TEST_ASSERT_INT(SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "LETER?X"));
    TEST_ASSERT(scrabble_rack_match(&rack, "LETTER", &match));
    TEST_ASSERT_INT(1, match.blanks_used);
    TEST_ASSERT_INT(1, match.blank_letter_counts['T' - 'A']);
    return 0;
}

static int clears_failed_match_results(void) {
    ScrabbleRack rack;
    ScrabbleRackMatch match;

    TEST_ASSERT_INT(SCRABBLE_RACK_OK, scrabble_rack_init(&rack, "ABC?"));
    memset(&match, 0xFF, sizeof(match));
    TEST_ASSERT(!scrabble_rack_match(&rack, "ZZ", &match));
    TEST_ASSERT_INT(0, match.blanks_used);
    for (size_t index = 0; index < SCRABBLE_ALPHABET_SIZE; ++index) {
        TEST_ASSERT_INT(0, match.blank_letter_counts[index]);
    }
    return 0;
}

static const ScrabbleTestCase TESTS[] = {
    {"initializes letters and blanks", initializes_letters_and_blanks},
    {"validates rack input", validates_rack_input},
    {"matches repeated letters", matches_repeated_letters},
    {"records blank substitutions", records_blank_substitutions},
    {"clears failed match results", clears_failed_match_results}
};

TEST_MAIN(TESTS)
