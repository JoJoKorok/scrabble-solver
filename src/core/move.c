#include "scrabble/move.h"

#include <string.h>

static int direction_is_valid(ScrabbleMoveDirection direction) {
    return direction == SCRABBLE_MOVE_HORIZONTAL ||
           direction == SCRABBLE_MOVE_VERTICAL;
}

static int normalize_letter(char letter, char *normalized) {
    unsigned char character = (unsigned char)letter;

    if (character >= 'a' && character <= 'z') {
        character = (unsigned char)(character - 'a' + 'A');
    }

    if (character < 'A' || character > 'Z') {
        return 0;
    }

    *normalized = (char)character;
    return 1;
}

static ScrabbleMoveTileMask tile_bit(size_t word_index) {
    return (ScrabbleMoveTileMask)(1u << word_index);
}

static int move_fits(
    ScrabbleBoardPosition start,
    ScrabbleMoveDirection direction,
    size_t length) {
    if (!scrabble_board_position_is_valid(start)) {
        return 0;
    }

    if (direction == SCRABBLE_MOVE_HORIZONTAL) {
        return length <= SCRABBLE_BOARD_SIZE - start.column;
    }

    return length <= SCRABBLE_BOARD_SIZE - start.row;
}

ScrabbleMoveStatus scrabble_move_init(
    ScrabbleMove *move,
    const char *word,
    ScrabbleBoardPosition start,
    ScrabbleMoveDirection direction) {
    ScrabbleMove candidate = {0};
    size_t length;

    if (move == NULL) {
        return SCRABBLE_MOVE_INVALID_ARGUMENT;
    }

    memset(move, 0, sizeof(*move));
    if (word == NULL) {
        return SCRABBLE_MOVE_INVALID_ARGUMENT;
    }

    if (!direction_is_valid(direction)) {
        return SCRABBLE_MOVE_INVALID_DIRECTION;
    }

    length = strlen(word);
    if (length == 0 || length > SCRABBLE_MAX_WORD_LENGTH) {
        return SCRABBLE_MOVE_INVALID_WORD;
    }

    if (!move_fits(start, direction, length)) {
        return SCRABBLE_MOVE_OUT_OF_BOUNDS;
    }

    for (size_t index = 0; index < length; ++index) {
        if (!normalize_letter(word[index], &candidate.word[index])) {
            return SCRABBLE_MOVE_INVALID_WORD;
        }
    }

    candidate.word[length] = '\0';
    candidate.start = start;
    candidate.direction = direction;
    candidate.length = length;
    candidate.rack_tile_mask =
        (ScrabbleMoveTileMask)((1u << length) - 1u);
    *move = candidate;
    return SCRABBLE_MOVE_OK;
}

ScrabbleMoveStatus scrabble_move_set_tile_from_rack(
    ScrabbleMove *move,
    size_t word_index,
    int from_rack) {
    ScrabbleMoveTileMask bit;

    if (move == NULL) {
        return SCRABBLE_MOVE_INVALID_ARGUMENT;
    }

    if (word_index >= move->length) {
        return SCRABBLE_MOVE_INVALID_TILE_INDEX;
    }

    bit = tile_bit(word_index);
    if (from_rack) {
        move->rack_tile_mask |= bit;
    } else {
        move->rack_tile_mask &= (ScrabbleMoveTileMask)~bit;
        move->blank_tile_mask &= (ScrabbleMoveTileMask)~bit;
    }

    return SCRABBLE_MOVE_OK;
}

ScrabbleMoveStatus scrabble_move_set_tile_blank(
    ScrabbleMove *move,
    size_t word_index,
    int is_blank) {
    ScrabbleMoveTileMask bit;

    if (move == NULL) {
        return SCRABBLE_MOVE_INVALID_ARGUMENT;
    }

    if (word_index >= move->length) {
        return SCRABBLE_MOVE_INVALID_TILE_INDEX;
    }

    bit = tile_bit(word_index);
    if (is_blank && (move->rack_tile_mask & bit) == 0) {
        return SCRABBLE_MOVE_TILE_NOT_FROM_RACK;
    }

    if (is_blank) {
        move->blank_tile_mask |= bit;
    } else {
        move->blank_tile_mask &= (ScrabbleMoveTileMask)~bit;
    }

    return SCRABBLE_MOVE_OK;
}

ScrabbleMoveStatus scrabble_move_tile_at(
    const ScrabbleMove *move,
    size_t word_index,
    ScrabbleMoveTile *tile) {
    ScrabbleMoveTileMask bit;

    if (tile != NULL) {
        memset(tile, 0, sizeof(*tile));
    }

    if (move == NULL || tile == NULL) {
        return SCRABBLE_MOVE_INVALID_ARGUMENT;
    }

    if (word_index >= move->length) {
        return SCRABBLE_MOVE_INVALID_TILE_INDEX;
    }

    bit = tile_bit(word_index);
    tile->position = move->start;
    if (move->direction == SCRABBLE_MOVE_HORIZONTAL) {
        tile->position.column += word_index;
    } else {
        tile->position.row += word_index;
    }
    tile->letter = move->word[word_index];
    tile->from_rack = (move->rack_tile_mask & bit) != 0;
    tile->is_blank = (move->blank_tile_mask & bit) != 0;
    return SCRABBLE_MOVE_OK;
}

size_t scrabble_move_rack_tile_count(const ScrabbleMove *move) {
    size_t count = 0;

    if (move == NULL) {
        return 0;
    }

    for (size_t index = 0; index < move->length; ++index) {
        if ((move->rack_tile_mask & tile_bit(index)) != 0) {
            ++count;
        }
    }

    return count;
}

int scrabble_move_covers_position(
    const ScrabbleMove *move,
    ScrabbleBoardPosition position) {
    if (move == NULL || !scrabble_board_position_is_valid(position)) {
        return 0;
    }

    if (move->direction == SCRABBLE_MOVE_HORIZONTAL) {
        return position.row == move->start.row &&
               position.column >= move->start.column &&
               position.column - move->start.column < move->length;
    }

    return position.column == move->start.column &&
           position.row >= move->start.row &&
           position.row - move->start.row < move->length;
}
