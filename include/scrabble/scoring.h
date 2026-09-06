#ifndef SCRABBLE_SCORING_H
#define SCRABBLE_SCORING_H

#include "scrabble/move.h"
#include "scrabble/rack.h"

typedef struct {
    /* Main-word letters before its word multiplier. */
    int letter_score;
    int word_multiplier;
    /* Fully multiplied total of every perpendicular word. */
    int cross_word_score;
    int bingo_bonus;
    int total_score;
} ScrabbleMoveScore;

typedef enum {
    SCRABBLE_SCORING_OK = 0,
    SCRABBLE_SCORING_INVALID_ARGUMENT,
    SCRABBLE_SCORING_INVALID_MOVE
} ScrabbleScoringStatus;

/* Returns the basic English-language tile score for an ASCII word.
   Returns -1 if word is NULL or contains a character outside A-Z. */
int scrabble_score_word(const char *word);

/* Scores a rack match, treating letters supplied by blank tiles as zero. */
int scrabble_score_rack_match(const char *word,
                              const ScrabbleRackMatch *match);

/* Scores a structurally validated opening move using permanent board
   premiums. Blanks score zero and seven newly placed tiles add 50 points. */
ScrabbleScoringStatus scrabble_score_opening_move(
    const ScrabbleMove *move,
    ScrabbleMoveScore *score);

/* Scores a resolved connected move against the board state before placement.
   Premiums affect only new rack tiles and apply to every word they form. */
ScrabbleScoringStatus scrabble_score_move(
    const ScrabbleBoard *board,
    const ScrabbleMove *move,
    ScrabbleMoveScore *score);

#endif
