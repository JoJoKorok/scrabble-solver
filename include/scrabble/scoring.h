#ifndef SCRABBLE_SCORING_H
#define SCRABBLE_SCORING_H

/* Returns the basic English-language tile score for an ASCII word.
   Returns -1 if word is NULL or contains a character outside A-Z. */
int scrabble_score_word(const char *word);

#endif
