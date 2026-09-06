#include "scrabble/placement.h"

#include "move_analysis.h"

#include <string.h>

static int letter_index(char letter) {
    return letter - 'A';
}

static ScrabblePlacementStatus canonicalize_move(
    const ScrabbleMove *move,
    ScrabbleMove *candidate) {
    ScrabbleMoveTileMask all_tiles;
    ScrabbleMoveStatus move_status;

    if (move->word[SCRABBLE_MAX_WORD_LENGTH] != '\0') {
        return SCRABBLE_PLACEMENT_INVALID_MOVE;
    }

    move_status = scrabble_move_init(
        candidate, move->word, move->start, move->direction);

    if (move_status != SCRABBLE_MOVE_OK ||
        move->length != candidate->length) {
        return SCRABBLE_PLACEMENT_INVALID_MOVE;
    }

    all_tiles = candidate->rack_tile_mask;
    if ((move->blank_tile_mask & (ScrabbleMoveTileMask)~all_tiles) != 0) {
        return SCRABBLE_PLACEMENT_INVALID_MOVE;
    }

    candidate->blank_tile_mask = move->blank_tile_mask;
    return SCRABBLE_PLACEMENT_OK;
}

static ScrabblePlacementStatus canonicalize_opening_move(
    const ScrabbleMove *move,
    ScrabbleMove *candidate) {
    ScrabblePlacementStatus status = canonicalize_move(move, candidate);

    if (status != SCRABBLE_PLACEMENT_OK || candidate->length < 2) {
        return SCRABBLE_PLACEMENT_INVALID_MOVE;
    }

    if (move->rack_tile_mask != candidate->rack_tile_mask) {
        return SCRABBLE_PLACEMENT_INVALID_MOVE;
    }

    return SCRABBLE_PLACEMENT_OK;
}

static int assign_and_consume_rack_tiles(
    const ScrabbleRack *rack,
    ScrabbleMove *move) {
    unsigned char available[SCRABBLE_ALPHABET_SIZE];
    unsigned char blanks_available = rack->blank_count;

    if (scrabble_move_rack_tile_count(move) > rack->tile_count) {
        return 0;
    }

    memcpy(available, rack->letter_counts, sizeof(available));
    for (size_t index = 0; index < move->length; ++index) {
        ScrabbleMoveTile tile;

        if (scrabble_move_tile_at(move, index, &tile) != SCRABBLE_MOVE_OK) {
            return 0;
        }

        if (!tile.from_rack) {
            continue;
        }

        if (tile.is_blank) {
            if (blanks_available == 0) {
                return 0;
            }
            --blanks_available;
        }
    }

    for (size_t index = 0; index < move->length; ++index) {
        ScrabbleMoveTile tile;
        int index_in_alphabet;

        if (scrabble_move_tile_at(move, index, &tile) != SCRABBLE_MOVE_OK) {
            return 0;
        }

        if (!tile.from_rack || tile.is_blank) {
            continue;
        }

        index_in_alphabet = letter_index(tile.letter);
        if (available[index_in_alphabet] > 0) {
            --available[index_in_alphabet];
        } else if (blanks_available > 0) {
            if (scrabble_move_set_tile_blank(move, index, 1) !=
                SCRABBLE_MOVE_OK) {
                return 0;
            }
            --blanks_available;
        } else {
            return 0;
        }
    }

    return 1;
}

ScrabblePlacementStatus scrabble_move_resolve_board_tiles(
    const ScrabbleBoard *board,
    const ScrabbleMove *move,
    ScrabbleMove *resolved_move) {
    ScrabbleMove proposal;
    ScrabbleMove candidate;
    ScrabblePlacementStatus status;
    size_t new_tile_count = 0;

    if (move != NULL) {
        proposal = *move;
    }
    if (resolved_move != NULL) {
        memset(resolved_move, 0, sizeof(*resolved_move));
    }

    if (board == NULL || move == NULL) {
        return SCRABBLE_PLACEMENT_INVALID_ARGUMENT;
    }

    status = canonicalize_move(&proposal, &candidate);
    if (status != SCRABBLE_PLACEMENT_OK) {
        return status;
    }

    for (size_t index = 0; index < candidate.length; ++index) {
        ScrabbleMoveTile tile;
        ScrabbleBoardCell cell;

        if (scrabble_move_tile_at(&candidate, index, &tile) !=
                SCRABBLE_MOVE_OK ||
            scrabble_board_get_cell(board, tile.position, &cell) !=
                SCRABBLE_BOARD_OK) {
            return SCRABBLE_PLACEMENT_INVALID_MOVE;
        }

        if (cell.letter == '\0') {
            ++new_tile_count;
        } else if (cell.letter != tile.letter) {
            return SCRABBLE_PLACEMENT_LETTER_CONFLICT;
        } else if (scrabble_move_set_tile_from_rack(
                       &candidate, index, 0) != SCRABBLE_MOVE_OK) {
            return SCRABBLE_PLACEMENT_INVALID_MOVE;
        }
    }

    if (new_tile_count == 0) {
        return SCRABBLE_PLACEMENT_NO_NEW_TILES;
    }

    if (resolved_move != NULL) {
        *resolved_move = candidate;
    }
    return SCRABBLE_PLACEMENT_OK;
}

ScrabblePlacementStatus scrabble_opening_move_validate(
    const ScrabbleBoard *board,
    const ScrabbleDictionary *dictionary,
    const ScrabbleRack *rack,
    const ScrabbleMove *move,
    ScrabbleMove *validated_move) {
    ScrabbleBoardPosition center = {
        SCRABBLE_BOARD_CENTER_INDEX,
        SCRABBLE_BOARD_CENTER_INDEX
    };
    ScrabbleMove proposal;
    ScrabbleMove candidate;
    ScrabblePlacementStatus status;

    if (move != NULL) {
        proposal = *move;
    }
    if (validated_move != NULL) {
        memset(validated_move, 0, sizeof(*validated_move));
    }

    if (board == NULL || dictionary == NULL || rack == NULL || move == NULL) {
        return SCRABBLE_PLACEMENT_INVALID_ARGUMENT;
    }

    if (!scrabble_board_is_empty(board)) {
        return SCRABBLE_PLACEMENT_BOARD_NOT_EMPTY;
    }

    status = canonicalize_opening_move(&proposal, &candidate);
    if (status != SCRABBLE_PLACEMENT_OK) {
        return status;
    }

    if (!scrabble_move_covers_position(&candidate, center)) {
        return SCRABBLE_PLACEMENT_MOVE_NOT_CENTERED;
    }

    if (!scrabble_dictionary_contains(dictionary, candidate.word)) {
        return SCRABBLE_PLACEMENT_WORD_NOT_IN_DICTIONARY;
    }

    if (!assign_and_consume_rack_tiles(rack, &candidate)) {
        return SCRABBLE_PLACEMENT_RACK_MISMATCH;
    }

    if (validated_move != NULL) {
        *validated_move = candidate;
    }
    return SCRABBLE_PLACEMENT_OK;
}

ScrabblePlacementStatus scrabble_opening_move_apply(
    ScrabbleBoard *board,
    const ScrabbleDictionary *dictionary,
    const ScrabbleRack *rack,
    const ScrabbleMove *move,
    ScrabbleMove *applied_move) {
    ScrabbleMove proposal;
    ScrabbleMove candidate;
    ScrabblePlacementStatus status;

    if (move != NULL) {
        proposal = *move;
    }
    if (applied_move != NULL) {
        memset(applied_move, 0, sizeof(*applied_move));
    }

    status = scrabble_opening_move_validate(
        board,
        dictionary,
        rack,
        move == NULL ? NULL : &proposal,
        &candidate);
    if (status != SCRABBLE_PLACEMENT_OK) {
        return status;
    }

    for (size_t index = 0; index < candidate.length; ++index) {
        ScrabbleMoveTile tile;

        if (scrabble_move_tile_at(&candidate, index, &tile) !=
                SCRABBLE_MOVE_OK ||
            scrabble_board_place_tile(
                board,
                tile.position,
                tile.letter,
                tile.is_blank) != SCRABBLE_BOARD_OK) {
            scrabble_board_clear(board);
            return SCRABBLE_PLACEMENT_BOARD_UPDATE_FAILED;
        }
    }

    if (applied_move != NULL) {
        *applied_move = candidate;
    }
    return SCRABBLE_PLACEMENT_OK;
}

ScrabblePlacementStatus scrabble_connected_move_validate(
    const ScrabbleBoard *board,
    const ScrabbleDictionary *dictionary,
    const ScrabbleRack *rack,
    const ScrabbleMove *move,
    ScrabbleMove *validated_move) {
    ScrabbleMove proposal;
    ScrabbleMove candidate;
    ScrabbleMoveAnalysis analysis;
    ScrabbleMoveAnalysisStatus analysis_status;
    ScrabblePlacementStatus status;

    if (move != NULL) {
        proposal = *move;
    }
    if (validated_move != NULL) {
        memset(validated_move, 0, sizeof(*validated_move));
    }

    if (board == NULL || dictionary == NULL || rack == NULL || move == NULL) {
        return SCRABBLE_PLACEMENT_INVALID_ARGUMENT;
    }
    if (scrabble_board_is_empty(board)) {
        return SCRABBLE_PLACEMENT_BOARD_EMPTY;
    }

    status = scrabble_move_resolve_board_tiles(board, &proposal, &candidate);
    if (status != SCRABBLE_PLACEMENT_OK) {
        return status;
    }

    analysis_status =
        scrabble_move_analyze_board(board, &candidate, &analysis);
    if (analysis_status == SCRABBLE_MOVE_ANALYSIS_INCOMPLETE_WORD) {
        return SCRABBLE_PLACEMENT_INCOMPLETE_WORD;
    }
    if (analysis_status != SCRABBLE_MOVE_ANALYSIS_OK) {
        return SCRABBLE_PLACEMENT_INVALID_MOVE;
    }
    if (!analysis.is_connected) {
        return SCRABBLE_PLACEMENT_MOVE_NOT_CONNECTED;
    }

    if (!scrabble_dictionary_contains(
            dictionary, analysis.words[0].text)) {
        return SCRABBLE_PLACEMENT_WORD_NOT_IN_DICTIONARY;
    }
    for (size_t index = 1; index < analysis.word_count; ++index) {
        if (!scrabble_dictionary_contains(
                dictionary, analysis.words[index].text)) {
            return SCRABBLE_PLACEMENT_CROSS_WORD_NOT_IN_DICTIONARY;
        }
    }

    if (!assign_and_consume_rack_tiles(rack, &candidate)) {
        return SCRABBLE_PLACEMENT_RACK_MISMATCH;
    }

    if (validated_move != NULL) {
        *validated_move = candidate;
    }
    return SCRABBLE_PLACEMENT_OK;
}

static void remove_placed_tiles(
    ScrabbleBoard *board,
    const ScrabbleMove *move,
    size_t placed_through_index) {
    for (size_t index = 0; index < placed_through_index; ++index) {
        ScrabbleMoveTile tile;

        if (scrabble_move_tile_at(move, index, &tile) ==
                SCRABBLE_MOVE_OK &&
            tile.from_rack) {
            scrabble_board_remove_tile(board, tile.position, NULL);
        }
    }
}

ScrabblePlacementStatus scrabble_connected_move_apply(
    ScrabbleBoard *board,
    const ScrabbleDictionary *dictionary,
    const ScrabbleRack *rack,
    const ScrabbleMove *move,
    ScrabbleMove *applied_move) {
    ScrabbleMove proposal;
    ScrabbleMove candidate;
    ScrabblePlacementStatus status;

    if (move != NULL) {
        proposal = *move;
    }
    if (applied_move != NULL) {
        memset(applied_move, 0, sizeof(*applied_move));
    }

    status = scrabble_connected_move_validate(
        board,
        dictionary,
        rack,
        move == NULL ? NULL : &proposal,
        &candidate);
    if (status != SCRABBLE_PLACEMENT_OK) {
        return status;
    }

    for (size_t index = 0; index < candidate.length; ++index) {
        ScrabbleMoveTile tile;

        if (scrabble_move_tile_at(&candidate, index, &tile) !=
            SCRABBLE_MOVE_OK) {
            remove_placed_tiles(board, &candidate, index);
            return SCRABBLE_PLACEMENT_BOARD_UPDATE_FAILED;
        }

        if (tile.from_rack &&
            scrabble_board_place_tile(
                board,
                tile.position,
                tile.letter,
                tile.is_blank) != SCRABBLE_BOARD_OK) {
            remove_placed_tiles(board, &candidate, index);
            return SCRABBLE_PLACEMENT_BOARD_UPDATE_FAILED;
        }
    }

    if (applied_move != NULL) {
        *applied_move = candidate;
    }
    return SCRABBLE_PLACEMENT_OK;
}
