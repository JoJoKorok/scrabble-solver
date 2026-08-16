#include "scrabble/game.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct ScrabbleGame {
    ScrabbleBoard *board;
    ScrabbleMove *moves;
    size_t move_count;
    size_t move_capacity;
};

static void set_placement_status(
    ScrabblePlacementStatus *status,
    ScrabblePlacementStatus value) {
    if (status != NULL) {
        *status = value;
    }
}

static int reserve_move(ScrabbleGame *game) {
    size_t new_capacity;
    void *new_moves;

    if (game->move_count < game->move_capacity) {
        return 1;
    }

    if (game->move_capacity > SIZE_MAX / 2) {
        return 0;
    }

    new_capacity = game->move_capacity == 0 ? 16 : game->move_capacity * 2;
    if (new_capacity > SIZE_MAX / sizeof(*game->moves)) {
        return 0;
    }

    new_moves = realloc(game->moves, new_capacity * sizeof(*game->moves));
    if (new_moves == NULL) {
        return 0;
    }

    game->moves = new_moves;
    game->move_capacity = new_capacity;
    return 1;
}

static int last_move_matches_board(const ScrabbleGame *game) {
    const ScrabbleMove *move = &game->moves[game->move_count - 1];

    for (size_t index = 0; index < move->length; ++index) {
        ScrabbleMoveTile tile;
        ScrabbleBoardCell cell;

        if (scrabble_move_tile_at(move, index, &tile) != SCRABBLE_MOVE_OK) {
            return 0;
        }

        if (!tile.from_rack) {
            continue;
        }

        if (scrabble_board_get_cell(game->board, tile.position, &cell) !=
                SCRABBLE_BOARD_OK ||
            cell.letter != tile.letter ||
            cell.is_blank != tile.is_blank) {
            return 0;
        }
    }

    return 1;
}

static void restore_removed_tiles(
    ScrabbleGame *game,
    const ScrabbleMove *move,
    size_t removed_through_index) {
    for (size_t index = 0; index < removed_through_index; ++index) {
        ScrabbleMoveTile tile;

        if (scrabble_move_tile_at(move, index, &tile) == SCRABBLE_MOVE_OK &&
            tile.from_rack) {
            scrabble_board_place_tile(
                game->board, tile.position, tile.letter, tile.is_blank);
        }
    }
}

ScrabbleGame *scrabble_game_create(void) {
    ScrabbleGame *game = calloc(1, sizeof(*game));

    if (game == NULL) {
        return NULL;
    }

    game->board = scrabble_board_create();
    if (game->board == NULL) {
        free(game);
        return NULL;
    }

    return game;
}

void scrabble_game_destroy(ScrabbleGame *game) {
    if (game != NULL) {
        scrabble_board_destroy(game->board);
        free(game->moves);
        free(game);
    }
}

void scrabble_game_reset(ScrabbleGame *game) {
    if (game != NULL) {
        scrabble_board_clear(game->board);
        game->move_count = 0;
    }
}

const ScrabbleBoard *scrabble_game_board(const ScrabbleGame *game) {
    return game == NULL ? NULL : game->board;
}

size_t scrabble_game_move_count(const ScrabbleGame *game) {
    return game == NULL ? 0 : game->move_count;
}

const ScrabbleMove *scrabble_game_move_at(
    const ScrabbleGame *game,
    size_t index) {
    if (game == NULL || index >= game->move_count) {
        return NULL;
    }

    return &game->moves[index];
}

ScrabbleGameStatus scrabble_game_apply_opening_move(
    ScrabbleGame *game,
    const ScrabbleDictionary *dictionary,
    const ScrabbleRack *rack,
    const ScrabbleMove *move,
    ScrabbleMove *applied_move,
    ScrabblePlacementStatus *placement_status) {
    ScrabbleMove proposal;
    ScrabbleMove validated;
    ScrabbleMove applied;
    ScrabblePlacementStatus status;

    if (move != NULL) {
        proposal = *move;
    }
    if (applied_move != NULL) {
        memset(applied_move, 0, sizeof(*applied_move));
    }
    set_placement_status(placement_status, SCRABBLE_PLACEMENT_OK);

    if (game == NULL || dictionary == NULL || rack == NULL || move == NULL) {
        set_placement_status(
            placement_status, SCRABBLE_PLACEMENT_INVALID_ARGUMENT);
        return SCRABBLE_GAME_INVALID_ARGUMENT;
    }

    status = scrabble_opening_move_validate(
        game->board, dictionary, rack, &proposal, &validated);
    if (status != SCRABBLE_PLACEMENT_OK) {
        set_placement_status(placement_status, status);
        return SCRABBLE_GAME_PLACEMENT_REJECTED;
    }

    if (!reserve_move(game)) {
        return SCRABBLE_GAME_OUT_OF_MEMORY;
    }

    status = scrabble_opening_move_apply(
        game->board, dictionary, rack, &validated, &applied);
    if (status != SCRABBLE_PLACEMENT_OK) {
        set_placement_status(placement_status, status);
        return SCRABBLE_GAME_BOARD_STATE_ERROR;
    }

    game->moves[game->move_count++] = applied;
    if (applied_move != NULL) {
        *applied_move = applied;
    }
    return SCRABBLE_GAME_OK;
}

ScrabbleGameStatus scrabble_game_undo_last_move(
    ScrabbleGame *game,
    ScrabbleMove *undone_move) {
    const ScrabbleMove *move;

    if (undone_move != NULL) {
        memset(undone_move, 0, sizeof(*undone_move));
    }

    if (game == NULL) {
        return SCRABBLE_GAME_INVALID_ARGUMENT;
    }

    if (game->move_count == 0) {
        return SCRABBLE_GAME_NO_MOVES;
    }

    if (!last_move_matches_board(game)) {
        return SCRABBLE_GAME_BOARD_STATE_ERROR;
    }

    move = &game->moves[game->move_count - 1];
    for (size_t index = 0; index < move->length; ++index) {
        ScrabbleMoveTile tile;

        if (scrabble_move_tile_at(move, index, &tile) != SCRABBLE_MOVE_OK) {
            restore_removed_tiles(game, move, index);
            return SCRABBLE_GAME_BOARD_STATE_ERROR;
        }

        if (tile.from_rack &&
            scrabble_board_remove_tile(game->board, tile.position, NULL) !=
                SCRABBLE_BOARD_OK) {
            restore_removed_tiles(game, move, index);
            return SCRABBLE_GAME_BOARD_STATE_ERROR;
        }
    }

    if (undone_move != NULL) {
        *undone_move = *move;
    }
    --game->move_count;
    return SCRABBLE_GAME_OK;
}
