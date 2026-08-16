#include "scrabble/board.h"

#include <stdlib.h>
#include <string.h>

struct ScrabbleBoard {
    ScrabbleBoardCell cells[SCRABBLE_BOARD_SIZE][SCRABBLE_BOARD_SIZE];
    size_t tile_count;
};

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

ScrabbleBoard *scrabble_board_create(void) {
    return calloc(1, sizeof(ScrabbleBoard));
}

void scrabble_board_destroy(ScrabbleBoard *board) {
    free(board);
}

void scrabble_board_clear(ScrabbleBoard *board) {
    if (board != NULL) {
        memset(board, 0, sizeof(*board));
    }
}

size_t scrabble_board_tile_count(const ScrabbleBoard *board) {
    return board == NULL ? 0 : board->tile_count;
}

int scrabble_board_is_empty(const ScrabbleBoard *board) {
    return board != NULL && board->tile_count == 0;
}

int scrabble_board_position_is_valid(ScrabbleBoardPosition position) {
    return position.row < SCRABBLE_BOARD_SIZE &&
           position.column < SCRABBLE_BOARD_SIZE;
}

int scrabble_board_position_is_center(ScrabbleBoardPosition position) {
    return position.row == SCRABBLE_BOARD_CENTER_INDEX &&
           position.column == SCRABBLE_BOARD_CENTER_INDEX;
}

ScrabbleBoardStatus scrabble_board_get_cell(
    const ScrabbleBoard *board,
    ScrabbleBoardPosition position,
    ScrabbleBoardCell *cell) {
    if (cell != NULL) {
        memset(cell, 0, sizeof(*cell));
    }

    if (board == NULL || cell == NULL) {
        return SCRABBLE_BOARD_INVALID_ARGUMENT;
    }

    if (!scrabble_board_position_is_valid(position)) {
        return SCRABBLE_BOARD_OUT_OF_BOUNDS;
    }

    *cell = board->cells[position.row][position.column];
    return SCRABBLE_BOARD_OK;
}

ScrabbleBoardStatus scrabble_board_place_tile(
    ScrabbleBoard *board,
    ScrabbleBoardPosition position,
    char letter,
    int is_blank) {
    ScrabbleBoardCell *cell;
    char normalized;

    if (board == NULL) {
        return SCRABBLE_BOARD_INVALID_ARGUMENT;
    }

    if (!scrabble_board_position_is_valid(position)) {
        return SCRABBLE_BOARD_OUT_OF_BOUNDS;
    }

    if (!normalize_letter(letter, &normalized)) {
        return SCRABBLE_BOARD_INVALID_LETTER;
    }

    cell = &board->cells[position.row][position.column];
    if (cell->letter != '\0') {
        return SCRABBLE_BOARD_CELL_OCCUPIED;
    }

    cell->letter = normalized;
    cell->is_blank = is_blank != 0;
    ++board->tile_count;
    return SCRABBLE_BOARD_OK;
}

ScrabbleBoardStatus scrabble_board_remove_tile(
    ScrabbleBoard *board,
    ScrabbleBoardPosition position,
    ScrabbleBoardCell *removed_cell) {
    ScrabbleBoardCell *cell;

    if (removed_cell != NULL) {
        memset(removed_cell, 0, sizeof(*removed_cell));
    }

    if (board == NULL) {
        return SCRABBLE_BOARD_INVALID_ARGUMENT;
    }

    if (!scrabble_board_position_is_valid(position)) {
        return SCRABBLE_BOARD_OUT_OF_BOUNDS;
    }

    cell = &board->cells[position.row][position.column];
    if (cell->letter == '\0') {
        return SCRABBLE_BOARD_CELL_EMPTY;
    }

    if (removed_cell != NULL) {
        *removed_cell = *cell;
    }
    memset(cell, 0, sizeof(*cell));
    --board->tile_count;
    return SCRABBLE_BOARD_OK;
}
