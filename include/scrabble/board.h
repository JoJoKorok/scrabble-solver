#ifndef SCRABBLE_BOARD_H
#define SCRABBLE_BOARD_H

#include "scrabble/constants.h"

#include <stddef.h>

typedef struct ScrabbleBoard ScrabbleBoard;

/* Board positions use zero-based row and column indexes. */
typedef struct {
    size_t row;
    size_t column;
} ScrabbleBoardPosition;

/* An empty cell has letter '\0'. A placed blank stores the letter it
   represents while is_blank remains nonzero for later scoring. */
typedef struct {
    char letter;
    unsigned char is_blank;
} ScrabbleBoardCell;

typedef enum {
    SCRABBLE_BOARD_OK = 0,
    SCRABBLE_BOARD_INVALID_ARGUMENT,
    SCRABBLE_BOARD_OUT_OF_BOUNDS,
    SCRABBLE_BOARD_INVALID_LETTER,
    SCRABBLE_BOARD_CELL_OCCUPIED
} ScrabbleBoardStatus;

/* Creates an owned, empty board. Returns NULL when allocation fails. */
ScrabbleBoard *scrabble_board_create(void);
void scrabble_board_destroy(ScrabbleBoard *board);

/* Removes every tile while retaining the board allocation. */
void scrabble_board_clear(ScrabbleBoard *board);

size_t scrabble_board_tile_count(const ScrabbleBoard *board);
int scrabble_board_is_empty(const ScrabbleBoard *board);

int scrabble_board_position_is_valid(ScrabbleBoardPosition position);
int scrabble_board_position_is_center(ScrabbleBoardPosition position);

/* Copies a cell into cell. The output is cleared when the lookup fails. */
ScrabbleBoardStatus scrabble_board_get_cell(
    const ScrabbleBoard *board,
    ScrabbleBoardPosition position,
    ScrabbleBoardCell *cell);

/* Places one resolved ASCII letter on an empty cell. This is a low-level
   board operation; whole-move validation is handled by the move layer. */
ScrabbleBoardStatus scrabble_board_place_tile(
    ScrabbleBoard *board,
    ScrabbleBoardPosition position,
    char letter,
    int is_blank);

#endif
