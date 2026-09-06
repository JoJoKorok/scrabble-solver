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
    SCRABBLE_PLACEMENT_LETTER_CONFLICT,
    SCRABBLE_PLACEMENT_NO_NEW_TILES,
    SCRABBLE_PLACEMENT_BOARD_EMPTY,
    SCRABBLE_PLACEMENT_MOVE_NOT_CONNECTED,
    SCRABBLE_PLACEMENT_INCOMPLETE_WORD,
    SCRABBLE_PLACEMENT_CROSS_WORD_NOT_IN_DICTIONARY,
    SCRABBLE_PLACEMENT_BOARD_UPDATE_FAILED
} ScrabblePlacementStatus;

/* Resolves a proposed word against the current board. Matching occupied
   cells are marked as existing board tiles; empty cells remain supplied by
   the rack. Blank identity for an existing tile remains stored on the board.
   This step does not validate connectivity, words, rack contents, or score.
   resolved_move may be NULL, and the board is never changed. */
ScrabblePlacementStatus scrabble_move_resolve_board_tiles(
    const ScrabbleBoard *board,
    const ScrabbleMove *move,
    ScrabbleMove *resolved_move);

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

/* Validates a move after the opening turn. Every formed word must be in the
   dictionary, the move must connect to the board, and only new tiles consume
   the rack. The board is never changed by validation. */
ScrabblePlacementStatus scrabble_connected_move_validate(
    const ScrabbleBoard *board,
    const ScrabbleDictionary *dictionary,
    const ScrabbleRack *rack,
    const ScrabbleMove *move,
    ScrabbleMove *validated_move);

/* Validates and atomically places a connected move. */
ScrabblePlacementStatus scrabble_connected_move_apply(
    ScrabbleBoard *board,
    const ScrabbleDictionary *dictionary,
    const ScrabbleRack *rack,
    const ScrabbleMove *move,
    ScrabbleMove *applied_move);

#endif
