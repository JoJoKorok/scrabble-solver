#ifndef SCRABBLE_MOVE_ANALYSIS_H
#define SCRABBLE_MOVE_ANALYSIS_H

#include "scrabble/board.h"
#include "scrabble/move.h"

#include <stddef.h>

#define SCRABBLE_MAX_FORMED_WORDS (SCRABBLE_MAX_WORD_LENGTH + 1)

typedef struct {
    char text[SCRABBLE_MAX_WORD_LENGTH + 1];
    ScrabbleBoardPosition start;
    ScrabbleMoveDirection direction;
    size_t length;
} ScrabbleFormedWord;

typedef struct {
    ScrabbleMove move;
    ScrabbleFormedWord words[SCRABBLE_MAX_FORMED_WORDS];
    size_t word_count;
    int is_connected;
} ScrabbleMoveAnalysis;

typedef enum {
    SCRABBLE_MOVE_ANALYSIS_OK = 0,
    SCRABBLE_MOVE_ANALYSIS_INVALID_ARGUMENT,
    SCRABBLE_MOVE_ANALYSIS_INVALID_MOVE,
    SCRABBLE_MOVE_ANALYSIS_INCOMPLETE_WORD
} ScrabbleMoveAnalysisStatus;

/* Analyzes a resolved move against the board state before it is applied. */
ScrabbleMoveAnalysisStatus scrabble_move_analyze_board(
    const ScrabbleBoard *board,
    const ScrabbleMove *move,
    ScrabbleMoveAnalysis *analysis);

/* Returns whether position is inside move and optionally copies its index. */
int scrabble_move_index_at_position(
    const ScrabbleMove *move,
    ScrabbleBoardPosition position,
    size_t *word_index);

#endif
