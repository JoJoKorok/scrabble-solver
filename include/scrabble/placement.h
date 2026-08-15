#ifndef SCRABBLE_PLACEMENT_H
#define SCRABBLE_PLACEMENT_H

#include "scrabble/board.h"
#include "scrabble/dictionary.h"
#include "scrabble/move.h"
#include "scrabble/rack.h"

typedef enum {
    SCRABBLE_PLACEMENT_OK = 0,
    SCRABBLE_PLACEMENT_INVALID_ARGUMENT,
    SCRABBLE_PLACEMENT_INVALID_MOVE,
    SCRABBLE_PLACEMENT_BOARD_NOT_EMPTY,
    SCRABBLE_PLACEMENT_MOVE_NOT_CENTERED,
    SCRABBLE_PLACEMENT_WORD_NOT_IN_DICTIONARY,
    SCRABBLE_PLACEMENT_RACK_MISMATCH,
    SCRABBLE_PLACEMENT_BOARD_UPDATE_FAILED
} ScrabblePlacementStatus;

/* Validates the special rules for the first move. validated_move may be NULL;
   otherwise it receives a copy with required blank tiles assigned. The board
   is never changed by validation. */
ScrabblePlacementStatus scrabble_opening_move_validate(
    const ScrabbleBoard *board,
    const ScrabbleDictionary *dictionary,
    const ScrabbleRack *rack,
    const ScrabbleMove *move,
    ScrabbleMove *validated_move);

/* Validates and atomically places the first word. applied_move may be NULL;
   otherwise it receives the applied move with blank assignments preserved. */
ScrabblePlacementStatus scrabble_opening_move_apply(
    ScrabbleBoard *board,
    const ScrabbleDictionary *dictionary,
    const ScrabbleRack *rack,
    const ScrabbleMove *move,
    ScrabbleMove *applied_move);

#endif
