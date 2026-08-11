#include "scrabble/rack.h"

#include <string.h>

static int letter_index(unsigned char character) {
    if (character >= 'a' && character <= 'z') {
        character = (unsigned char)(character - 'a' + 'A');
    }

    if (character < 'A' || character > 'Z') {
        return -1;
    }

    return character - 'A';
}

ScrabbleRackStatus scrabble_rack_init(ScrabbleRack *rack, const char *tiles) {
    size_t tile_count;

    if (rack == NULL || tiles == NULL) {
        return SCRABBLE_RACK_INVALID_ARGUMENT;
    }

    memset(rack, 0, sizeof(*rack));
    tile_count = strlen(tiles);

    if (tile_count > SCRABBLE_RACK_CAPACITY) {
        return SCRABBLE_RACK_TOO_MANY_TILES;
    }

    for (size_t index = 0; index < tile_count; ++index) {
        int alphabet_index = letter_index((unsigned char)tiles[index]);

        if (alphabet_index < 0) {
            memset(rack, 0, sizeof(*rack));
            return SCRABBLE_RACK_INVALID_CHARACTER;
        }

        ++rack->letter_counts[alphabet_index];
    }

    rack->tile_count = tile_count;
    return SCRABBLE_RACK_OK;
}

int scrabble_rack_can_form(const ScrabbleRack *rack, const char *word) {
    unsigned char available[SCRABBLE_ALPHABET_SIZE];
    size_t word_length;

    if (rack == NULL || word == NULL) {
        return 0;
    }

    word_length = strlen(word);
    if (word_length == 0 || word_length > rack->tile_count) {
        return 0;
    }

    memcpy(available, rack->letter_counts, sizeof(available));

    for (size_t index = 0; index < word_length; ++index) {
        int alphabet_index = letter_index((unsigned char)word[index]);

        if (alphabet_index < 0 || available[alphabet_index] == 0) {
            return 0;
        }

        --available[alphabet_index];
    }

    return 1;
}
