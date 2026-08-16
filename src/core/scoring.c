#include "scrabble/scoring.h"

#include <string.h>

static const int TILE_VALUES[26] = {
    1, 3, 3, 2, 1, 4, 2, 4, 1, 8, 5, 1, 3,
    1, 1, 3, 10, 1, 1, 1, 1, 4, 4, 8, 4, 10
};

static int tile_value(unsigned char letter) {
    if (letter >= 'a' && letter <= 'z') {
        letter = (unsigned char)(letter - 'a' + 'A');
    }

    if (letter < 'A' || letter > 'Z') {
        return -1;
    }

    return TILE_VALUES[letter - 'A'];
}

int scrabble_score_word(const char *word) {
    int score = 0;

    if (word == 0) {
        return -1;
    }

    for (; *word != '\0'; ++word) {
        unsigned char letter = (unsigned char)*word;
        int value;

        if (letter >= 'a' && letter <= 'z') {
            letter = (unsigned char)(letter - 'a' + 'A');
        }
        value = tile_value(letter);

        if (value < 0) {
            return -1;
        }

        score += value;
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
        int value;

        if (letter >= 'a' && letter <= 'z') {
            letter = (unsigned char)(letter - 'a' + 'A');
        }
        value = tile_value(letter);

        if (value < 0) {
            return -1;
        }

        if (blanks[letter - 'A'] > 0) {
            --blanks[letter - 'A'];
            ++blanks_consumed;
        } else {
            score += value;
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

static int canonicalize_opening_move(
    const ScrabbleMove *move,
    ScrabbleMove *candidate) {
    ScrabbleMoveTileMask all_tiles;

    if (scrabble_move_init(
            candidate, move->word, move->start, move->direction) !=
            SCRABBLE_MOVE_OK ||
        candidate->length != move->length ||
        candidate->length < 2 ||
        candidate->length > SCRABBLE_RACK_CAPACITY) {
        return 0;
    }

    all_tiles = candidate->rack_tile_mask;
    if (move->rack_tile_mask != all_tiles ||
        (move->blank_tile_mask & (ScrabbleMoveTileMask)~all_tiles) != 0) {
        return 0;
    }

    candidate->blank_tile_mask = move->blank_tile_mask;
    return 1;
}

ScrabbleScoringStatus scrabble_score_opening_move(
    const ScrabbleMove *move,
    ScrabbleMoveScore *score) {
    ScrabbleBoardPosition center = {
        SCRABBLE_BOARD_CENTER_INDEX,
        SCRABBLE_BOARD_CENTER_INDEX
    };
    ScrabbleMove candidate;
    ScrabbleMoveScore result = {0, 1, 0, 0};

    if (score != NULL) {
        memset(score, 0, sizeof(*score));
    }

    if (move == NULL || score == NULL) {
        return SCRABBLE_SCORING_INVALID_ARGUMENT;
    }

    if (!canonicalize_opening_move(move, &candidate) ||
        !scrabble_move_covers_position(&candidate, center)) {
        return SCRABBLE_SCORING_INVALID_MOVE;
    }

    for (size_t index = 0; index < candidate.length; ++index) {
        ScrabbleMoveTile tile;
        ScrabbleBoardPremium premium;
        int value;

        if (scrabble_move_tile_at(&candidate, index, &tile) !=
                SCRABBLE_MOVE_OK ||
            scrabble_board_get_premium(tile.position, &premium) !=
                SCRABBLE_BOARD_OK) {
            return SCRABBLE_SCORING_INVALID_MOVE;
        }

        value = tile.is_blank ? 0 : tile_value((unsigned char)tile.letter);
        if (value < 0) {
            return SCRABBLE_SCORING_INVALID_MOVE;
        }

        if (premium == SCRABBLE_BOARD_DOUBLE_LETTER) {
            value *= 2;
        } else if (premium == SCRABBLE_BOARD_TRIPLE_LETTER) {
            value *= 3;
        } else if (premium == SCRABBLE_BOARD_DOUBLE_WORD) {
            result.word_multiplier *= 2;
        } else if (premium == SCRABBLE_BOARD_TRIPLE_WORD) {
            result.word_multiplier *= 3;
        }
        result.letter_score += value;
    }

    if (scrabble_move_rack_tile_count(&candidate) ==
        SCRABBLE_RACK_CAPACITY) {
        result.bingo_bonus = 50;
    }
    result.total_score =
        result.letter_score * result.word_multiplier + result.bingo_bonus;
    *score = result;
    return SCRABBLE_SCORING_OK;
}
