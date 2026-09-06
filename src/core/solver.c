#include "scrabble/solver.h"

#include "dictionary_internal.h"
#include "scrabble/placement.h"
#include "scrabble/scoring.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    ScrabbleBoardPosition position;
    char letter;
} ScrabbleSolverAnchor;

typedef struct {
    unsigned short letter_counts[SCRABBLE_ALPHABET_SIZE];
    unsigned char blank_count;
} ScrabbleSolverInventory;

static int compare_results(const void *left_pointer, const void *right_pointer) {
    const ScrabbleResult *left = left_pointer;
    const ScrabbleResult *right = right_pointer;
    size_t left_length;
    size_t right_length;

    if (left->score != right->score) {
        return left->score < right->score ? 1 : -1;
    }

    left_length = strlen(left->word);
    right_length = strlen(right->word);
    if (left_length != right_length) {
        return left_length < right_length ? 1 : -1;
    }

    {
        int word_order = strcmp(left->word, right->word);

        if (word_order != 0) {
            return word_order;
        }
    }

    if (left->has_placement != right->has_placement) {
        return left->has_placement ? -1 : 1;
    }
    if (!left->has_placement) {
        return 0;
    }
    if (left->move.start.row != right->move.start.row) {
        return left->move.start.row < right->move.start.row ? -1 : 1;
    }
    if (left->move.start.column != right->move.start.column) {
        return left->move.start.column < right->move.start.column ? -1 : 1;
    }
    if (left->move.direction != right->move.direction) {
        return left->move.direction < right->move.direction ? -1 : 1;
    }
    return 0;
}

static int reserve_result(
    ScrabbleResult **items,
    size_t count,
    size_t *capacity) {
    if (count == *capacity) {
        size_t new_capacity;
        void *new_items;

        if (*capacity > SIZE_MAX / 2) {
            return 0;
        }

        new_capacity = *capacity == 0 ? 64 : *capacity * 2;
        if (new_capacity > SIZE_MAX / sizeof(**items)) {
            return 0;
        }

        new_items = realloc(*items, new_capacity * sizeof(**items));
        if (new_items == NULL) {
            return 0;
        }

        *items = new_items;
        *capacity = new_capacity;
    }

    return 1;
}

static int append_result(ScrabbleResult **items, size_t *count,
                         size_t *capacity, const char *word,
                         const ScrabbleRackMatch *match) {
    ScrabbleResult *result;

    if (!reserve_result(items, *count, capacity)) {
        return 0;
    }

    result = &(*items)[*count];
    memset(result, 0, sizeof(*result));
    strcpy(result->word, word);
    result->score = scrabble_score_rack_match(word, match);
    result->rack_match = *match;
    ++*count;
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

static int has_adjacent_board_tile(
    const ScrabbleBoard *board,
    ScrabbleBoardPosition position) {
    ScrabbleBoardPosition adjacent;

    if (position.row > 0) {
        adjacent = position;
        --adjacent.row;
        if (board_cell_is_occupied(board, adjacent)) {
            return 1;
        }
    }
    if (position.row + 1 < SCRABBLE_BOARD_SIZE) {
        adjacent = position;
        ++adjacent.row;
        if (board_cell_is_occupied(board, adjacent)) {
            return 1;
        }
    }
    if (position.column > 0) {
        adjacent = position;
        --adjacent.column;
        if (board_cell_is_occupied(board, adjacent)) {
            return 1;
        }
    }
    if (position.column + 1 < SCRABBLE_BOARD_SIZE) {
        adjacent = position;
        ++adjacent.column;
        if (board_cell_is_occupied(board, adjacent)) {
            return 1;
        }
    }

    return 0;
}

static size_t collect_anchors(
    const ScrabbleBoard *board,
    ScrabbleSolverAnchor anchors[SCRABBLE_BOARD_SIZE * SCRABBLE_BOARD_SIZE]) {
    size_t count = 0;

    for (size_t row = 0; row < SCRABBLE_BOARD_SIZE; ++row) {
        for (size_t column = 0; column < SCRABBLE_BOARD_SIZE; ++column) {
            ScrabbleBoardPosition position = {row, column};
            ScrabbleBoardCell cell;

            if (scrabble_board_get_cell(board, position, &cell) !=
                SCRABBLE_BOARD_OK) {
                continue;
            }
            if (cell.letter == '\0' &&
                !has_adjacent_board_tile(board, position)) {
                continue;
            }

            anchors[count].position = position;
            anchors[count].letter = cell.letter;
            ++count;
        }
    }

    return count;
}

static void build_inventory(
    const ScrabbleBoard *board,
    const ScrabbleRack *rack,
    ScrabbleSolverInventory *inventory) {
    memset(inventory, 0, sizeof(*inventory));
    for (size_t index = 0; index < SCRABBLE_ALPHABET_SIZE; ++index) {
        inventory->letter_counts[index] = rack->letter_counts[index];
    }
    inventory->blank_count = rack->blank_count;

    for (size_t row = 0; row < SCRABBLE_BOARD_SIZE; ++row) {
        for (size_t column = 0; column < SCRABBLE_BOARD_SIZE; ++column) {
            ScrabbleBoardPosition position = {row, column};
            ScrabbleBoardCell cell;

            if (scrabble_board_get_cell(board, position, &cell) ==
                    SCRABBLE_BOARD_OK &&
                cell.letter != '\0') {
                ++inventory->letter_counts[cell.letter - 'A'];
            }
        }
    }
}

static int inventory_can_form(
    const ScrabbleSolverInventory *inventory,
    const char *word) {
    unsigned short available[SCRABBLE_ALPHABET_SIZE];
    unsigned char blanks_available = inventory->blank_count;

    memcpy(available, inventory->letter_counts, sizeof(available));
    for (; *word != '\0'; ++word) {
        size_t letter_index = (size_t)(*word - 'A');

        if (available[letter_index] > 0) {
            --available[letter_index];
        } else if (blanks_available > 0) {
            --blanks_available;
        } else {
            return 0;
        }
    }

    return 1;
}

static void mark_candidate_starts(
    const ScrabbleSolverAnchor *anchors,
    size_t anchor_count,
    const char *word,
    size_t word_length,
    unsigned char candidates[2][SCRABBLE_BOARD_SIZE][SCRABBLE_BOARD_SIZE]) {
    for (size_t anchor_index = 0;
         anchor_index < anchor_count;
         ++anchor_index) {
        const ScrabbleSolverAnchor *anchor = &anchors[anchor_index];

        for (size_t word_index = 0;
             word_index < word_length;
             ++word_index) {
            size_t start;

            if (anchor->letter != '\0' &&
                anchor->letter != word[word_index]) {
                continue;
            }

            if (anchor->position.column >= word_index) {
                start = anchor->position.column - word_index;
                if (word_length <= SCRABBLE_BOARD_SIZE - start) {
                    candidates[SCRABBLE_MOVE_HORIZONTAL]
                              [anchor->position.row][start] = 1;
                }
            }

            if (anchor->position.row >= word_index) {
                start = anchor->position.row - word_index;
                if (word_length <= SCRABBLE_BOARD_SIZE - start) {
                    candidates[SCRABBLE_MOVE_VERTICAL]
                              [start][anchor->position.column] = 1;
                }
            }
        }
    }
}

static size_t tile_mask_count(ScrabbleMoveTileMask mask) {
    size_t count = 0;

    while (mask != 0) {
        count += mask & 1u;
        mask >>= 1;
    }

    return count;
}

static int find_best_blank_assignment(
    const ScrabbleDictionary *dictionary,
    const ScrabbleRack *rack,
    const ScrabbleBoard *board,
    const ScrabbleMove *proposal,
    ScrabbleMove *best_move,
    ScrabbleMoveScore *best_score) {
    ScrabbleMove validated;
    ScrabbleMove candidate;
    ScrabbleMoveScore candidate_score;
    ScrabbleMoveTileMask rack_tiles;
    ScrabbleMoveTileMask subset;
    size_t blanks_used;

    if (scrabble_connected_move_validate(
            board, dictionary, rack, proposal, &validated) !=
            SCRABBLE_PLACEMENT_OK ||
        scrabble_score_move(board, &validated, best_score) !=
            SCRABBLE_SCORING_OK) {
        return 0;
    }

    *best_move = validated;
    blanks_used = tile_mask_count(validated.blank_tile_mask);
    if (blanks_used == 0) {
        return 1;
    }

    rack_tiles = validated.rack_tile_mask;
    subset = rack_tiles;
    for (;;) {
        if (tile_mask_count(subset) == blanks_used) {
            ScrabbleMove alternative = validated;

            alternative.blank_tile_mask = subset;
            if (scrabble_connected_move_validate(
                    board,
                    dictionary,
                    rack,
                    &alternative,
                    &candidate) == SCRABBLE_PLACEMENT_OK &&
                scrabble_score_move(
                    board, &candidate, &candidate_score) ==
                    SCRABBLE_SCORING_OK &&
                (candidate_score.total_score > best_score->total_score ||
                 (candidate_score.total_score == best_score->total_score &&
                  candidate.blank_tile_mask < best_move->blank_tile_mask))) {
                *best_move = candidate;
                *best_score = candidate_score;
            }
        }

        if (subset == 0) {
            break;
        }
        subset = (ScrabbleMoveTileMask)((subset - 1u) & rack_tiles);
    }

    return 1;
}

static void set_result_rack_match(
    ScrabbleResult *result,
    const ScrabbleMove *move) {
    for (size_t index = 0; index < move->length; ++index) {
        ScrabbleMoveTile tile;

        if (scrabble_move_tile_at(move, index, &tile) ==
                SCRABBLE_MOVE_OK &&
            tile.from_rack && tile.is_blank) {
            ++result->rack_match.blank_letter_counts[tile.letter - 'A'];
            ++result->rack_match.blanks_used;
        }
    }
}

static int append_board_result(
    ScrabbleResult **items,
    size_t *count,
    size_t *capacity,
    const ScrabbleMove *move,
    const ScrabbleMoveScore *score) {
    ScrabbleResult *result;

    if (!reserve_result(items, *count, capacity)) {
        return 0;
    }

    result = &(*items)[*count];
    memset(result, 0, sizeof(*result));
    strcpy(result->word, move->word);
    result->score = score->total_score;
    result->move = *move;
    result->has_placement = 1;
    set_result_rack_match(result, move);
    ++*count;
    return 1;
}

ScrabbleSolverStatus scrabble_solve(
    const ScrabbleDictionary *dictionary,
    const ScrabbleRack *rack,
    ScrabbleResultSet *results) {
    ScrabbleResult *items;
    size_t capacity = 0;
    size_t candidate_count;
    size_t result_count = 0;

    if (results == NULL) {
        return SCRABBLE_SOLVER_INVALID_ARGUMENT;
    }

    results->items = NULL;
    results->count = 0;
    if (dictionary == NULL || rack == NULL) {
        return SCRABBLE_SOLVER_INVALID_ARGUMENT;
    }

    candidate_count = scrabble_dictionary_candidate_count(
        dictionary, rack->tile_count);
    if (candidate_count == 0) {
        return SCRABBLE_SOLVER_OK;
    }

    items = NULL;

    for (size_t index = 0; index < candidate_count; ++index) {
        const char *word = scrabble_dictionary_candidate_word_at(
            dictionary, rack->tile_count, index);
        ScrabbleRackMatch match;

        if (!scrabble_rack_match(rack, word, &match)) {
            continue;
        }

        if (!append_result(&items, &result_count, &capacity, word, &match)) {
            free(items);
            return SCRABBLE_SOLVER_OUT_OF_MEMORY;
        }
    }

    if (result_count == 0) {
        free(items);
        return SCRABBLE_SOLVER_OK;
    }

    if (result_count > 1) {
        qsort(items, result_count, sizeof(*items), compare_results);
    }
    results->items = items;
    results->count = result_count;
    return SCRABBLE_SOLVER_OK;
}

ScrabbleSolverStatus scrabble_solve_board(
    const ScrabbleDictionary *dictionary,
    const ScrabbleRack *rack,
    const ScrabbleBoard *board,
    ScrabbleResultSet *results) {
    ScrabbleSolverAnchor
        anchors[SCRABBLE_BOARD_SIZE * SCRABBLE_BOARD_SIZE];
    ScrabbleResult *items = NULL;
    ScrabbleSolverInventory inventory;
    size_t anchor_count;
    size_t capacity = 0;
    size_t result_count = 0;

    if (results == NULL) {
        return SCRABBLE_SOLVER_INVALID_ARGUMENT;
    }

    results->items = NULL;
    results->count = 0;
    if (dictionary == NULL || rack == NULL || board == NULL) {
        return SCRABBLE_SOLVER_INVALID_ARGUMENT;
    }
    if (scrabble_board_is_empty(board) || rack->tile_count == 0) {
        return SCRABBLE_SOLVER_OK;
    }

    anchor_count = collect_anchors(board, anchors);
    build_inventory(board, rack, &inventory);
    for (size_t word_index = 0;
         word_index < scrabble_dictionary_count(dictionary);
         ++word_index) {
        const char *word = scrabble_dictionary_word_at(
            dictionary, word_index);
        size_t word_length = strlen(word);
        unsigned char candidates
            [2][SCRABBLE_BOARD_SIZE][SCRABBLE_BOARD_SIZE] = {{{0}}};

        if (word_length < 2 || word_length > SCRABBLE_MAX_WORD_LENGTH ||
            !inventory_can_form(&inventory, word)) {
            continue;
        }

        mark_candidate_starts(
            anchors,
            anchor_count,
            word,
            word_length,
            candidates);

        for (int direction = SCRABBLE_MOVE_HORIZONTAL;
             direction <= SCRABBLE_MOVE_VERTICAL;
             ++direction) {
            for (size_t row = 0; row < SCRABBLE_BOARD_SIZE; ++row) {
                for (size_t column = 0;
                     column < SCRABBLE_BOARD_SIZE;
                     ++column) {
                    ScrabbleBoardPosition start = {row, column};
                    ScrabbleMove proposal;
                    ScrabbleMove best_move;
                    ScrabbleMoveScore best_score;

                    if (!candidates[direction][row][column] ||
                        scrabble_move_init(
                            &proposal,
                            word,
                            start,
                            (ScrabbleMoveDirection)direction) !=
                            SCRABBLE_MOVE_OK ||
                        !find_best_blank_assignment(
                            dictionary,
                            rack,
                            board,
                            &proposal,
                            &best_move,
                            &best_score)) {
                        continue;
                    }

                    if (!append_board_result(
                            &items,
                            &result_count,
                            &capacity,
                            &best_move,
                            &best_score)) {
                        free(items);
                        return SCRABBLE_SOLVER_OUT_OF_MEMORY;
                    }
                }
            }
        }
    }

    if (result_count > 1) {
        qsort(items, result_count, sizeof(*items), compare_results);
    }
    results->items = items;
    results->count = result_count;
    return SCRABBLE_SOLVER_OK;
}

void scrabble_result_set_destroy(ScrabbleResultSet *results) {
    if (results != NULL) {
        free(results->items);
        results->items = NULL;
        results->count = 0;
    }
}
