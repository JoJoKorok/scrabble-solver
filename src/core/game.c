#include "scrabble/game.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct ScrabbleGame {
    ScrabbleBoard *board;
    ScrabbleGameTurn *turns;
    size_t move_count;
    size_t turn_capacity;
};

static void set_placement_status(
    ScrabblePlacementStatus *status,
    ScrabblePlacementStatus value) {
    if (status != NULL) {
        *status = value;
    }
}

static int reserve_turn(ScrabbleGame *game) {
    size_t new_capacity;
    void *new_turns;

    if (game->move_count < game->turn_capacity) {
        return 1;
    }

    if (game->turn_capacity > SIZE_MAX / 2) {
        return 0;
    }

    new_capacity = game->turn_capacity == 0 ? 16 : game->turn_capacity * 2;
    if (new_capacity > SIZE_MAX / sizeof(*game->turns)) {
        return 0;
    }

    new_turns = realloc(game->turns, new_capacity * sizeof(*game->turns));
    if (new_turns == NULL) {
        return 0;
    }

    game->turns = new_turns;
    game->turn_capacity = new_capacity;
    return 1;
}

static int infer_visible_tile_rack(
    const ScrabbleMove *move,
    ScrabbleRack *rack) {
    char tiles[SCRABBLE_RACK_CAPACITY + 1] = {0};
    size_t tile_count = 0;

    for (size_t index = 0; index < move->length; ++index) {
        ScrabbleMoveTile tile;

        if (scrabble_move_tile_at(move, index, &tile) != SCRABBLE_MOVE_OK) {
            return 0;
        }
        if (!tile.from_rack) {
            continue;
        }
        if (tile_count >= SCRABBLE_RACK_CAPACITY) {
            return 0;
        }
        tiles[tile_count] = tile.is_blank ? '?' : tile.letter;
        ++tile_count;
    }

    return tile_count > 0 &&
        scrabble_rack_init(rack, tiles) == SCRABBLE_RACK_OK;
}

static int last_move_matches_board(const ScrabbleGame *game) {
    const ScrabbleMove *move = &game->turns[game->move_count - 1].move;

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
        free(game->turns);
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

    return &game->turns[index].move;
}

const ScrabbleGameTurn *scrabble_game_turn_at(
    const ScrabbleGame *game,
    size_t index) {
    if (game == NULL || index >= game->move_count) {
        return NULL;
    }

    return &game->turns[index];
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
    ScrabbleMoveScore score;
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

    if (scrabble_score_opening_move(&validated, &score) !=
        SCRABBLE_SCORING_OK) {
        return SCRABBLE_GAME_BOARD_STATE_ERROR;
    }

    if (!reserve_turn(game)) {
        return SCRABBLE_GAME_OUT_OF_MEMORY;
    }

    status = scrabble_opening_move_apply(
        game->board, dictionary, rack, &validated, &applied);
    if (status != SCRABBLE_PLACEMENT_OK) {
        set_placement_status(placement_status, status);
        return SCRABBLE_GAME_BOARD_STATE_ERROR;
    }

    game->turns[game->move_count].move = applied;
    game->turns[game->move_count].score = score;
    game->turns[game->move_count].owner = SCRABBLE_TURN_OWNER_USER;
    ++game->move_count;
    if (applied_move != NULL) {
        *applied_move = applied;
    }
    return SCRABBLE_GAME_OK;
}

ScrabbleGameStatus scrabble_game_apply_move(
    ScrabbleGame *game,
    const ScrabbleDictionary *dictionary,
    const ScrabbleRack *rack,
    const ScrabbleMove *move,
    ScrabbleMove *applied_move,
    ScrabblePlacementStatus *placement_status) {
    ScrabbleMove proposal;
    ScrabbleMove validated;
    ScrabbleMove applied;
    ScrabbleMoveScore score;
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

    status = scrabble_connected_move_validate(
        game->board, dictionary, rack, &proposal, &validated);
    if (status != SCRABBLE_PLACEMENT_OK) {
        set_placement_status(placement_status, status);
        return SCRABBLE_GAME_PLACEMENT_REJECTED;
    }

    if (scrabble_score_move(game->board, &validated, &score) !=
        SCRABBLE_SCORING_OK) {
        return SCRABBLE_GAME_BOARD_STATE_ERROR;
    }

    if (!reserve_turn(game)) {
        return SCRABBLE_GAME_OUT_OF_MEMORY;
    }

    status = scrabble_connected_move_apply(
        game->board, dictionary, rack, &validated, &applied);
    if (status != SCRABBLE_PLACEMENT_OK) {
        set_placement_status(placement_status, status);
        return SCRABBLE_GAME_BOARD_STATE_ERROR;
    }

    game->turns[game->move_count].move = applied;
    game->turns[game->move_count].score = score;
    game->turns[game->move_count].owner = SCRABBLE_TURN_OWNER_USER;
    ++game->move_count;
    if (applied_move != NULL) {
        *applied_move = applied;
    }
    return SCRABBLE_GAME_OK;
}

ScrabbleGameStatus scrabble_game_record_opponent_move(
    ScrabbleGame *game,
    const ScrabbleDictionary *dictionary,
    const ScrabbleMove *move,
    ScrabbleMove *applied_move,
    ScrabblePlacementStatus *placement_status) {
    ScrabbleMove proposal;
    ScrabbleMove validated;
    ScrabbleMove applied;
    ScrabbleMoveScore score;
    ScrabbleRack visible_tiles;
    ScrabblePlacementStatus status;
    int is_opening;

    if (move != NULL) {
        proposal = *move;
    }
    if (applied_move != NULL) {
        memset(applied_move, 0, sizeof(*applied_move));
    }
    set_placement_status(placement_status, SCRABBLE_PLACEMENT_OK);

    if (game == NULL || dictionary == NULL || move == NULL) {
        set_placement_status(
            placement_status, SCRABBLE_PLACEMENT_INVALID_ARGUMENT);
        return SCRABBLE_GAME_INVALID_ARGUMENT;
    }

    is_opening = game->move_count == 0;
    status = is_opening
        ? scrabble_opening_move_validate_board(
              game->board, dictionary, &proposal, &validated)
        : scrabble_connected_move_validate_board(
              game->board, dictionary, &proposal, &validated);
    if (status != SCRABBLE_PLACEMENT_OK) {
        set_placement_status(placement_status, status);
        return SCRABBLE_GAME_PLACEMENT_REJECTED;
    }
    if (!infer_visible_tile_rack(&validated, &visible_tiles)) {
        set_placement_status(
            placement_status, SCRABBLE_PLACEMENT_INVALID_MOVE);
        return SCRABBLE_GAME_PLACEMENT_REJECTED;
    }

    if ((is_opening &&
         scrabble_score_opening_move(&validated, &score) !=
             SCRABBLE_SCORING_OK) ||
        (!is_opening &&
         scrabble_score_move(game->board, &validated, &score) !=
             SCRABBLE_SCORING_OK)) {
        return SCRABBLE_GAME_BOARD_STATE_ERROR;
    }

    if (!reserve_turn(game)) {
        return SCRABBLE_GAME_OUT_OF_MEMORY;
    }

    status = is_opening
        ? scrabble_opening_move_apply(
              game->board,
              dictionary,
              &visible_tiles,
              &validated,
              &applied)
        : scrabble_connected_move_apply(
              game->board,
              dictionary,
              &visible_tiles,
              &validated,
              &applied);
    if (status != SCRABBLE_PLACEMENT_OK) {
        set_placement_status(placement_status, status);
        return SCRABBLE_GAME_BOARD_STATE_ERROR;
    }

    game->turns[game->move_count].move = applied;
    game->turns[game->move_count].score = score;
    game->turns[game->move_count].owner = SCRABBLE_TURN_OWNER_OPPONENT;
    ++game->move_count;
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

    move = &game->turns[game->move_count - 1].move;
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
