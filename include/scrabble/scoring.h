#ifndef SCRABBLE_SCORING_H
#define SCRABBLE_SCORING_H

#include "scrabble/rack.h"

/* Returns the basic English-language tile score for an ASCII word.
   Returns -1 if word is NULL or contains a character outside A-Z. */
int scrabble_score_word(const char *word);

/* Scores a rack match, treating letters supplied by blank tiles as zero. */
int scrabble_score_rack_match(const char *word,
                              const ScrabbleRackMatch *match);

#endif
