#include "scrabble/scoring.h"

#include <string.h>

static const int TILE_VALUES[26] = {
    1, 3, 3, 2, 1, 4, 2, 4, 1, 8, 5, 1, 3,
    1, 1, 3, 10, 1, 1, 1, 1, 4, 4, 8, 4, 10
};

int scrabble_score_word(const char *word) {
    int score = 0;

    if (word == 0) {
        return -1;
    }

    for (; *word != '\0'; ++word) {
        unsigned char letter = (unsigned char)*word;

        if (letter >= 'a' && letter <= 'z') {
            letter = (unsigned char)(letter - 'a' + 'A');
        }

        if (letter < 'A' || letter > 'Z') {
            return -1;
        }

        score += TILE_VALUES[letter - 'A'];
    }

    return score;
}

int scrabble_score_rack_match(const char *word,
                              const ScrabbleRackMatch *match) {
    unsigned char blanks[SCRABBLE_ALPHABET_SIZE];
    size_t blanks_consumed = 0;
    int score = 0;

    if (word == NULL || match == NULL) {
        return -1;
    }

    memcpy(blanks, match->blank_letter_counts, sizeof(blanks));

    for (; *word != '\0'; ++word) {
        unsigned char letter = (unsigned char)*word;

        if (letter >= 'a' && letter <= 'z') {
            letter = (unsigned char)(letter - 'a' + 'A');
        }

        if (letter < 'A' || letter > 'Z') {
            return -1;
        }

        if (blanks[letter - 'A'] > 0) {
            --blanks[letter - 'A'];
            ++blanks_consumed;
        } else {
            score += TILE_VALUES[letter - 'A'];
        }
    }

    for (size_t index = 0; index < SCRABBLE_ALPHABET_SIZE; ++index) {
        if (blanks[index] != 0) {
            return -1;
        }
    }

    if (blanks_consumed != match->blanks_used) {
        return -1;
    }

    return score;
}
