#include "scrabble/solver.h"

#include "scrabble/scoring.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

    return strcmp(left->word, right->word);
}

static int append_result(ScrabbleResult **items, size_t *count,
                         size_t *capacity, const char *word,
                         const ScrabbleRackMatch *match) {
    if (*count == *capacity) {
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

    strcpy((*items)[*count].word, word);
    (*items)[*count].score = scrabble_score_rack_match(word, match);
    (*items)[*count].rack_match = *match;
    ++*count;
    return 1;
}

ScrabbleSolverStatus scrabble_solve(
    const ScrabbleDictionary *dictionary,
    const ScrabbleRack *rack,
    ScrabbleResultSet *results) {
    ScrabbleResult *items;
    size_t capacity = 0;
    size_t dictionary_count;
    size_t result_count = 0;

    if (results == NULL) {
        return SCRABBLE_SOLVER_INVALID_ARGUMENT;
    }

    results->items = NULL;
    results->count = 0;
    if (dictionary == NULL || rack == NULL) {
        return SCRABBLE_SOLVER_INVALID_ARGUMENT;
    }

    dictionary_count = scrabble_dictionary_count(dictionary);
    if (dictionary_count == 0) {
        return SCRABBLE_SOLVER_OK;
    }

    items = NULL;

    for (size_t index = 0; index < dictionary_count; ++index) {
        const char *word = scrabble_dictionary_word_at(dictionary, index);
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

void scrabble_result_set_destroy(ScrabbleResultSet *results) {
    if (results != NULL) {
        free(results->items);
        results->items = NULL;
        results->count = 0;
    }
}
