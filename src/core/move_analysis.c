#include "move_analysis.h"

#include <string.h>

static int positions_equal(
    ScrabbleBoardPosition left,
    ScrabbleBoardPosition right) {
    return left.row == right.row && left.column == right.column;
}

static int step_position(
    ScrabbleBoardPosition position,
    ScrabbleMoveDirection direction,
    int forward,
    ScrabbleBoardPosition *next) {
    *next = position;

    if (direction == SCRABBLE_MOVE_HORIZONTAL) {
        if (forward) {
            if (position.column + 1 >= SCRABBLE_BOARD_SIZE) {
                return 0;
            }
            ++next->column;
        } else {
            if (position.column == 0) {
                return 0;
            }
            --next->column;
        }
    } else {
        if (forward) {
            if (position.row + 1 >= SCRABBLE_BOARD_SIZE) {
                return 0;
            }
            ++next->row;
        } else {
            if (position.row == 0) {
                return 0;
            }
            --next->row;
        }
    }

    return 1;
}

static int board_cell_is_occupied(
    const ScrabbleBoard *board,
    ScrabbleBoardPosition position) {
    ScrabbleBoardCell cell;

    return scrabble_board_get_cell(board, position, &cell) ==
               SCRABBLE_BOARD_OK &&
           cell.letter != '\0';
}

int scrabble_move_index_at_position(
    const ScrabbleMove *move,
    ScrabbleBoardPosition position,
    size_t *word_index) {
    size_t index;

    if (move == NULL || !scrabble_board_position_is_valid(position)) {
        return 0;
    }

    if (move->direction == SCRABBLE_MOVE_HORIZONTAL) {
        if (position.row != move->start.row ||
            position.column < move->start.column) {
            return 0;
        }
        index = position.column - move->start.column;
    } else if (move->direction == SCRABBLE_MOVE_VERTICAL) {
        if (position.column != move->start.column ||
            position.row < move->start.row) {
            return 0;
        }
        index = position.row - move->start.row;
    } else {
        return 0;
    }

    if (index >= move->length) {
        return 0;
    }

    if (word_index != NULL) {
        *word_index = index;
    }
    return 1;
}

static int canonicalize_resolved_move(
    const ScrabbleMove *move,
    ScrabbleMove *candidate) {
    ScrabbleMoveTileMask all_tiles;

    if (move->word[SCRABBLE_MAX_WORD_LENGTH] != '\0' ||
        scrabble_move_init(
            candidate, move->word, move->start, move->direction) !=
            SCRABBLE_MOVE_OK ||
        candidate->length != move->length ||
        candidate->length < 2) {
        return 0;
    }

    all_tiles = candidate->rack_tile_mask;
    if ((move->rack_tile_mask & (ScrabbleMoveTileMask)~all_tiles) != 0 ||
        (move->blank_tile_mask &
         (ScrabbleMoveTileMask)~move->rack_tile_mask) != 0 ||
        move->rack_tile_mask == 0) {
        return 0;
    }

    candidate->rack_tile_mask = move->rack_tile_mask;
    candidate->blank_tile_mask = move->blank_tile_mask;
    return 1;
}

static int has_adjacent_board_tile(
    const ScrabbleBoard *board,
    ScrabbleBoardPosition position) {
    static const int ROW_OFFSETS[] = {-1, 1, 0, 0};
    static const int COLUMN_OFFSETS[] = {0, 0, -1, 1};

    for (size_t index = 0; index < 4; ++index) {
        long row = (long)position.row + ROW_OFFSETS[index];
        long column = (long)position.column + COLUMN_OFFSETS[index];
        ScrabbleBoardPosition adjacent;

        if (row < 0 || column < 0 ||
            row >= SCRABBLE_BOARD_SIZE ||
            column >= SCRABBLE_BOARD_SIZE) {
            continue;
        }

        adjacent.row = (size_t)row;
        adjacent.column = (size_t)column;
        if (board_cell_is_occupied(board, adjacent)) {
            return 1;
        }
    }

    return 0;
}

static int main_word_is_complete(
    const ScrabbleBoard *board,
    const ScrabbleMove *move) {
    ScrabbleMoveTile last_tile;
    ScrabbleBoardPosition outside;

    if (step_position(move->start, move->direction, 0, &outside) &&
        board_cell_is_occupied(board, outside)) {
        return 0;
    }

    if (scrabble_move_tile_at(move, move->length - 1, &last_tile) !=
        SCRABBLE_MOVE_OK) {
        return 0;
    }

    return !step_position(
               last_tile.position, move->direction, 1, &outside) ||
           !board_cell_is_occupied(board, outside);
}

static int append_cross_word(
    const ScrabbleBoard *board,
    const ScrabbleMoveTile *tile,
    ScrabbleMoveAnalysis *analysis) {
    ScrabbleMoveDirection direction =
        analysis->move.direction == SCRABBLE_MOVE_HORIZONTAL
            ? SCRABBLE_MOVE_VERTICAL
            : SCRABBLE_MOVE_HORIZONTAL;
    ScrabbleBoardPosition start = tile->position;
    ScrabbleBoardPosition end = tile->position;
    ScrabbleBoardPosition next;
    ScrabbleBoardPosition current;
    ScrabbleFormedWord *word;

    while (step_position(start, direction, 0, &next) &&
           board_cell_is_occupied(board, next)) {
        start = next;
    }
    while (step_position(end, direction, 1, &next) &&
           board_cell_is_occupied(board, next)) {
        end = next;
    }

    if (positions_equal(start, end)) {
        return 1;
    }
    if (analysis->word_count >= SCRABBLE_MAX_FORMED_WORDS) {
        return 0;
    }

    word = &analysis->words[analysis->word_count];
    word->start = start;
    word->direction = direction;
    current = start;

    for (;;) {
        ScrabbleBoardCell cell;

        if (word->length >= SCRABBLE_MAX_WORD_LENGTH) {
            return 0;
        }

        if (positions_equal(current, tile->position)) {
            word->text[word->length] = tile->letter;
        } else {
            if (scrabble_board_get_cell(board, current, &cell) !=
                    SCRABBLE_BOARD_OK ||
                cell.letter == '\0') {
                return 0;
            }
            word->text[word->length] = cell.letter;
        }
        ++word->length;

        if (positions_equal(current, end)) {
            break;
        }
        if (!step_position(current, direction, 1, &current)) {
            return 0;
        }
    }

    word->text[word->length] = '\0';
    ++analysis->word_count;
    return 1;
}

ScrabbleMoveAnalysisStatus scrabble_move_analyze_board(
    const ScrabbleBoard *board,
    const ScrabbleMove *move,
    ScrabbleMoveAnalysis *analysis) {
    ScrabbleMove candidate;

    if (analysis != NULL) {
        memset(analysis, 0, sizeof(*analysis));
    }
    if (board == NULL || move == NULL || analysis == NULL) {
        return SCRABBLE_MOVE_ANALYSIS_INVALID_ARGUMENT;
    }

    if (!canonicalize_resolved_move(move, &candidate)) {
        return SCRABBLE_MOVE_ANALYSIS_INVALID_MOVE;
    }

    analysis->move = candidate;
    analysis->words[0].start = candidate.start;
    analysis->words[0].direction = candidate.direction;
    analysis->words[0].length = candidate.length;
    memcpy(
        analysis->words[0].text,
        candidate.word,
        candidate.length + 1);
    analysis->word_count = 1;

    if (!main_word_is_complete(board, &candidate)) {
        memset(analysis, 0, sizeof(*analysis));
        return SCRABBLE_MOVE_ANALYSIS_INCOMPLETE_WORD;
    }

    for (size_t index = 0; index < candidate.length; ++index) {
        ScrabbleMoveTile tile;
        ScrabbleBoardCell cell;

        if (scrabble_move_tile_at(&candidate, index, &tile) !=
                SCRABBLE_MOVE_OK ||
            scrabble_board_get_cell(board, tile.position, &cell) !=
                SCRABBLE_BOARD_OK) {
            memset(analysis, 0, sizeof(*analysis));
            return SCRABBLE_MOVE_ANALYSIS_INVALID_MOVE;
        }

        if (tile.from_rack) {
            if (cell.letter != '\0' ||
                !append_cross_word(board, &tile, analysis)) {
                memset(analysis, 0, sizeof(*analysis));
                return SCRABBLE_MOVE_ANALYSIS_INVALID_MOVE;
            }
            if (has_adjacent_board_tile(board, tile.position)) {
                analysis->is_connected = 1;
            }
        } else {
            if (cell.letter != tile.letter) {
                memset(analysis, 0, sizeof(*analysis));
                return SCRABBLE_MOVE_ANALYSIS_INVALID_MOVE;
            }
            analysis->is_connected = 1;
        }
    }

    return SCRABBLE_MOVE_ANALYSIS_OK;
}
