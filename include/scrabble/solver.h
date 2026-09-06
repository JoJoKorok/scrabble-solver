#ifndef SCRABBLE_SOLVER_H
#define SCRABBLE_SOLVER_H

#include "scrabble/board.h"
#include "scrabble/dictionary.h"
#include "scrabble/move.h"
#include "scrabble/rack.h"

#include <stddef.h>

typedef struct {
    char word[SCRABBLE_MAX_WORD_LENGTH + 1];
    int score;
    ScrabbleRackMatch rack_match;
    ScrabbleMove move;
    unsigned char has_placement;
} ScrabbleResult;

typedef struct {
    ScrabbleResult *items;
    size_t count;
} ScrabbleResultSet;

typedef enum {
    SCRABBLE_SOLVER_OK = 0,
    SCRABBLE_SOLVER_INVALID_ARGUMENT,
    SCRABBLE_SOLVER_OUT_OF_MEMORY
} ScrabbleSolverStatus;

/* Finds every dictionary word that the rack can form. Results are ordered by
   score (highest first), length (longest first), then alphabetically. */
ScrabbleSolverStatus scrabble_solve(
    const ScrabbleDictionary *dictionary,
    const ScrabbleRack *rack,
    ScrabbleResultSet *results);

/* Finds every legal connected placement on a non-empty board. Results retain
   exact placement and blank-tile details and are ordered by score, word
   length, word, position, then direction. An empty board has no connected
   results; use scrabble_solve for opening-word choices. */
ScrabbleSolverStatus scrabble_solve_board(
    const ScrabbleDictionary *dictionary,
    const ScrabbleRack *rack,
    const ScrabbleBoard *board,
    ScrabbleResultSet *results);

void scrabble_result_set_destroy(ScrabbleResultSet *results);

#endif
