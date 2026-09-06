#ifndef SCRABBLE_GAME_H
#define SCRABBLE_GAME_H

#include "scrabble/board.h"
#include "scrabble/dictionary.h"
#include "scrabble/move.h"
#include "scrabble/placement.h"
#include "scrabble/rack.h"
#include "scrabble/scoring.h"

#include <stddef.h>

typedef struct ScrabbleGame ScrabbleGame;

typedef struct {
    ScrabbleMove move;
    ScrabbleMoveScore score;
} ScrabbleGameTurn;

typedef enum {
    SCRABBLE_GAME_OK = 0,
    SCRABBLE_GAME_INVALID_ARGUMENT,
    SCRABBLE_GAME_PLACEMENT_REJECTED,
    SCRABBLE_GAME_OUT_OF_MEMORY,
    SCRABBLE_GAME_NO_MOVES,
    SCRABBLE_GAME_BOARD_STATE_ERROR
} ScrabbleGameStatus;

/* Creates an owned game containing an empty board and move history. */
ScrabbleGame *scrabble_game_create(void);
void scrabble_game_destroy(ScrabbleGame *game);

/* Clears the board and history for a new game. */
void scrabble_game_reset(ScrabbleGame *game);

/* The returned board and history entries remain owned by the game and must
   not be modified or released by the caller. */
const ScrabbleBoard *scrabble_game_board(const ScrabbleGame *game);
size_t scrabble_game_move_count(const ScrabbleGame *game);
const ScrabbleMove *scrabble_game_move_at(
    const ScrabbleGame *game,
    size_t index);
const ScrabbleGameTurn *scrabble_game_turn_at(
    const ScrabbleGame *game,
    size_t index);

/* Applies and records the first move. placement_status may be NULL; otherwise
   it reports the detailed placement result. applied_move may also be NULL. */
ScrabbleGameStatus scrabble_game_apply_opening_move(
    ScrabbleGame *game,
    const ScrabbleDictionary *dictionary,
    const ScrabbleRack *rack,
    const ScrabbleMove *move,
    ScrabbleMove *applied_move,
    ScrabblePlacementStatus *placement_status);

/* Applies and records a connected move after the opening turn. */
ScrabbleGameStatus scrabble_game_apply_move(
    ScrabbleGame *game,
    const ScrabbleDictionary *dictionary,
    const ScrabbleRack *rack,
    const ScrabbleMove *move,
    ScrabbleMove *applied_move,
    ScrabblePlacementStatus *placement_status);

/* Removes only the rack tiles introduced by the most recent move. */
ScrabbleGameStatus scrabble_game_undo_last_move(
    ScrabbleGame *game,
    ScrabbleMove *undone_move);

#endif
