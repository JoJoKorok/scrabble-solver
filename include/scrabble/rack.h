#ifndef SCRABBLE_RACK_H
#define SCRABBLE_RACK_H

#include <stddef.h>

#define SCRABBLE_RACK_CAPACITY 7
#define SCRABBLE_ALPHABET_SIZE 26

typedef struct {
    unsigned char letter_counts[SCRABBLE_ALPHABET_SIZE];
    size_t tile_count;
} ScrabbleRack;

typedef enum {
    SCRABBLE_RACK_OK = 0,
    SCRABBLE_RACK_INVALID_ARGUMENT,
    SCRABBLE_RACK_INVALID_CHARACTER,
    SCRABBLE_RACK_TOO_MANY_TILES
} ScrabbleRackStatus;

/* Initializes a rack from zero to seven ASCII letters. */
ScrabbleRackStatus scrabble_rack_init(ScrabbleRack *rack, const char *tiles);

/* Returns 1 if the rack contains every tile required by word, otherwise 0. */
int scrabble_rack_can_form(const ScrabbleRack *rack, const char *word);

#endif
