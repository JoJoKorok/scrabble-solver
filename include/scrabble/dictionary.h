#ifndef SCRABBLE_DICTIONARY_H
#define SCRABBLE_DICTIONARY_H

#include "scrabble/constants.h"

#include <stddef.h>

typedef struct ScrabbleDictionary ScrabbleDictionary;

typedef enum {
    SCRABBLE_DICTIONARY_OK = 0,
    SCRABBLE_DICTIONARY_INVALID_ARGUMENT,
    SCRABBLE_DICTIONARY_OPEN_FAILED,
    SCRABBLE_DICTIONARY_READ_FAILED,
    SCRABBLE_DICTIONARY_OUT_OF_MEMORY
} ScrabbleDictionaryStatus;

/* Loads alphabetic words from a one-word-per-line text file. Invalid lines
   are ignored, words are normalized to uppercase, and duplicates are removed. */
ScrabbleDictionary *scrabble_dictionary_load(
    const char *path,
    ScrabbleDictionaryStatus *status);

void scrabble_dictionary_destroy(ScrabbleDictionary *dictionary);
size_t scrabble_dictionary_count(const ScrabbleDictionary *dictionary);

/* Returns a dictionary-owned word, or NULL when index is out of range. */
const char *scrabble_dictionary_word_at(
    const ScrabbleDictionary *dictionary,
    size_t index);

/* Returns 1 when word is present. Lookup is case-insensitive for supported
   ASCII letters and returns 0 for invalid input. */
int scrabble_dictionary_contains(
    const ScrabbleDictionary *dictionary,
    const char *word);

#endif
