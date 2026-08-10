#include "scrabble/scoring.h"

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
