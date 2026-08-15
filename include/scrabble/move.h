#ifndef SCRABBLE_MOVE_H
#define SCRABBLE_MOVE_H

#include "scrabble/board.h"
#include "scrabble/constants.h"

#include <stddef.h>
#include <stdint.h>

typedef enum {
    SCRABBLE_MOVE_HORIZONTAL = 0,
    SCRABBLE_MOVE_VERTICAL
} ScrabbleMoveDirection;

typedef uint16_t ScrabbleMoveTileMask;

typedef struct {
    ScrabbleBoardPosition position;
    char letter;
    unsigned char from_rack;
    unsigned char is_blank;
} ScrabbleMoveTile;

/* A move describes one complete word and which of its letters are newly
   supplied by the rack. Board validation later marks existing intersections
   as not coming from the rack. */
typedef struct {
    ScrabbleBoardPosition start;
    ScrabbleMoveDirection direction;
    char word[SCRABBLE_MAX_WORD_LENGTH + 1];
    size_t length;
    ScrabbleMoveTileMask rack_tile_mask;
    ScrabbleMoveTileMask blank_tile_mask;
} ScrabbleMove;

typedef enum {
    SCRABBLE_MOVE_OK = 0,
    SCRABBLE_MOVE_INVALID_ARGUMENT,
    SCRABBLE_MOVE_INVALID_DIRECTION,
    SCRABBLE_MOVE_INVALID_WORD,
    SCRABBLE_MOVE_OUT_OF_BOUNDS,
    SCRABBLE_MOVE_INVALID_TILE_INDEX,
    SCRABBLE_MOVE_TILE_NOT_FROM_RACK
} ScrabbleMoveStatus;

/* Initializes a move and normalizes its word to uppercase. Every letter is
   initially marked as supplied by the rack. The move must fit on the board. */
ScrabbleMoveStatus scrabble_move_init(
    ScrabbleMove *move,
    const char *word,
    ScrabbleBoardPosition start,
    ScrabbleMoveDirection direction);

/* Marks whether a word position uses a newly placed rack tile. Marking an
   existing board tile also clears any blank marker for that position. */
ScrabbleMoveStatus scrabble_move_set_tile_from_rack(
    ScrabbleMove *move,
    size_t word_index,
    int from_rack);

/* Marks a newly placed tile as blank. Existing board tiles cannot be changed
   into blanks by a move. */
ScrabbleMoveStatus scrabble_move_set_tile_blank(
    ScrabbleMove *move,
    size_t word_index,
    int is_blank);

/* Copies the resolved letter, position, and tile source at word_index. */
ScrabbleMoveStatus scrabble_move_tile_at(
    const ScrabbleMove *move,
    size_t word_index,
    ScrabbleMoveTile *tile);

size_t scrabble_move_rack_tile_count(const ScrabbleMove *move);
int scrabble_move_covers_position(
    const ScrabbleMove *move,
    ScrabbleBoardPosition position);

#endif
