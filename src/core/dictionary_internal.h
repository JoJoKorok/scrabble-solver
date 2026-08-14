#ifndef SCRABBLE_DICTIONARY_INTERNAL_H
#define SCRABBLE_DICTIONARY_INTERNAL_H

#include "scrabble/dictionary.h"

#include <stddef.h>

/* Private candidate view used by the solver. Words are grouped by length and
   limited to those that can fit on a rack of max_length tiles. */
size_t scrabble_dictionary_candidate_count(
    const ScrabbleDictionary *dictionary,
    size_t max_length);

const char *scrabble_dictionary_candidate_word_at(
    const ScrabbleDictionary *dictionary,
    size_t max_length,
    size_t index);

#endif
