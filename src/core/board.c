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

static int is_triple_word_position(ScrabbleBoardPosition position) {
    int row_is_edge_or_middle = position.row == 0 || position.row == 7 ||
                                position.row == 14;
    int column_is_edge_or_middle = position.column == 0 ||
                                   position.column == 7 ||
                                   position.column == 14;

    return row_is_edge_or_middle && column_is_edge_or_middle &&
           !scrabble_board_position_is_center(position);
}

static int is_double_word_position(ScrabbleBoardPosition position) {
    size_t row = position.row;
    size_t column = position.column;
    int is_inner_diagonal =
        (row >= 1 && row <= 4) || (row >= 10 && row <= 13);

    return scrabble_board_position_is_center(position) ||
           (is_inner_diagonal &&
            (row == column || row + column == SCRABBLE_BOARD_SIZE - 1));
}

static int is_triple_letter_position(ScrabbleBoardPosition position) {
    size_t row = position.row;
    size_t column = position.column;

    if (row == 1 || row == 13) {
        return column == 5 || column == 9;
    }

    if (row == 5 || row == 9) {
        return column == 1 || column == 5 || column == 9 || column == 13;
    }

    return 0;
}

static int is_double_letter_position(ScrabbleBoardPosition position) {
    size_t row = position.row;
    size_t column = position.column;

    if (row == 0 || row == 14) {
        return column == 3 || column == 11;
    }
    if (row == 2 || row == 12) {
        return column == 6 || column == 8;
    }
    if (row == 3 || row == 11) {
        return column == 0 || column == 7 || column == 14;
    }
    if (row == 6 || row == 8) {
        return column == 2 || column == 6 || column == 8 || column == 12;
    }
    if (row == 7) {
        return column == 3 || column == 11;
    }

    return 0;
}

ScrabbleBoardStatus scrabble_board_get_premium(
    ScrabbleBoardPosition position,
    ScrabbleBoardPremium *premium) {
    if (premium != NULL) {
        *premium = SCRABBLE_BOARD_PREMIUM_NONE;
    }

    if (premium == NULL) {
        return SCRABBLE_BOARD_INVALID_ARGUMENT;
    }

    if (!scrabble_board_position_is_valid(position)) {
        return SCRABBLE_BOARD_OUT_OF_BOUNDS;
    }

    if (is_triple_word_position(position)) {
        *premium = SCRABBLE_BOARD_TRIPLE_WORD;
    } else if (is_double_word_position(position)) {
        *premium = SCRABBLE_BOARD_DOUBLE_WORD;
    } else if (is_triple_letter_position(position)) {
        *premium = SCRABBLE_BOARD_TRIPLE_LETTER;
    } else if (is_double_letter_position(position)) {
        *premium = SCRABBLE_BOARD_DOUBLE_LETTER;
    }

    return SCRABBLE_BOARD_OK;
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
