#include "scrabble/dictionary.h"

#include "dictionary_internal.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ScrabbleDictionary {
    char (*words)[SCRABBLE_MAX_WORD_LENGTH + 1];
    size_t *candidate_indices;
    size_t candidate_end_offsets[SCRABBLE_RACK_CAPACITY + 1];
    size_t count;
    size_t capacity;
};

static void set_status(ScrabbleDictionaryStatus *status,
                       ScrabbleDictionaryStatus value) {
    if (status != NULL) {
        *status = value;
    }
}

static int compare_words(const void *left, const void *right) {
    return strcmp((const char *)left, (const char *)right);
}

static int normalize_word(char *word) {
    size_t length = strlen(word);

    if (length == 0 || length > SCRABBLE_MAX_WORD_LENGTH) {
        return 0;
    }

    for (size_t index = 0; index < length; ++index) {
        unsigned char letter = (unsigned char)word[index];

        if (letter >= 'a' && letter <= 'z') {
            word[index] = (char)(letter - 'a' + 'A');
        } else if (letter < 'A' || letter > 'Z') {
            return 0;
        }
    }

    return 1;
}

static int append_word(ScrabbleDictionary *dictionary, const char *word) {
    if (dictionary->count == dictionary->capacity) {
        size_t new_capacity;
        void *new_words;

        if (dictionary->capacity > SIZE_MAX / 2) {
            return 0;
        }

        new_capacity = dictionary->capacity == 0
            ? 256
            : dictionary->capacity * 2;

        if (new_capacity > SIZE_MAX / sizeof(*dictionary->words)) {
            return 0;
        }

        new_words = realloc(dictionary->words,
                            new_capacity * sizeof(*dictionary->words));

        if (new_words == NULL) {
            return 0;
        }

        dictionary->words = new_words;
        dictionary->capacity = new_capacity;
    }

    strcpy(dictionary->words[dictionary->count], word);
    ++dictionary->count;
    return 1;
}

static void remove_duplicates(ScrabbleDictionary *dictionary) {
    size_t unique_count = 0;

    for (size_t index = 0; index < dictionary->count; ++index) {
        if (unique_count == 0 ||
            strcmp(dictionary->words[index],
                   dictionary->words[unique_count - 1]) != 0) {
            if (unique_count != index) {
                size_t word_size = strlen(dictionary->words[index]) + 1;
                memmove(dictionary->words[unique_count],
                        dictionary->words[index], word_size);
            }
            ++unique_count;
        }
    }

    dictionary->count = unique_count;
}

static int build_candidate_index(ScrabbleDictionary *dictionary) {
    size_t counts[SCRABBLE_RACK_CAPACITY + 1] = {0};
    size_t positions[SCRABBLE_RACK_CAPACITY + 1] = {0};
    size_t candidate_count = 0;

    for (size_t index = 0; index < dictionary->count; ++index) {
        size_t length = strlen(dictionary->words[index]);

        if (length <= SCRABBLE_RACK_CAPACITY) {
            ++counts[length];
            ++candidate_count;
        }
    }

    if (candidate_count > SIZE_MAX / sizeof(*dictionary->candidate_indices)) {
        return 0;
    }

    if (candidate_count > 0) {
        dictionary->candidate_indices = malloc(
            candidate_count * sizeof(*dictionary->candidate_indices));
        if (dictionary->candidate_indices == NULL) {
            return 0;
        }
    }

    for (size_t length = 1; length <= SCRABBLE_RACK_CAPACITY; ++length) {
        positions[length] = dictionary->candidate_end_offsets[length - 1];
        dictionary->candidate_end_offsets[length] =
            positions[length] + counts[length];
    }

    for (size_t index = 0; index < dictionary->count; ++index) {
        size_t length = strlen(dictionary->words[index]);

        if (length <= SCRABBLE_RACK_CAPACITY) {
            dictionary->candidate_indices[positions[length]++] = index;
        }
    }

    return 1;
}

ScrabbleDictionary *scrabble_dictionary_load(
    const char *path,
    ScrabbleDictionaryStatus *status) {
    ScrabbleDictionary *dictionary;
    FILE *file;
    char line[SCRABBLE_MAX_WORD_LENGTH + 3];

    set_status(status, SCRABBLE_DICTIONARY_OK);
    if (path == NULL) {
        set_status(status, SCRABBLE_DICTIONARY_INVALID_ARGUMENT);
        return NULL;
    }

    file = fopen(path, "r");
    if (file == NULL) {
        set_status(status, SCRABBLE_DICTIONARY_OPEN_FAILED);
        return NULL;
    }

    dictionary = calloc(1, sizeof(*dictionary));
    if (dictionary == NULL) {
        fclose(file);
        set_status(status, SCRABBLE_DICTIONARY_OUT_OF_MEMORY);
        return NULL;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        size_t length = strlen(line);
        int complete_line = length > 0 && line[length - 1] == '\n';

        if (complete_line) {
            line[--length] = '\0';
            if (length > 0 && line[length - 1] == '\r') {
                line[--length] = '\0';
            }
        } else if (!feof(file)) {
            int character;
            while ((character = fgetc(file)) != '\n' && character != EOF) {
            }
            continue;
        }

        if (normalize_word(line) && !append_word(dictionary, line)) {
            scrabble_dictionary_destroy(dictionary);
            fclose(file);
            set_status(status, SCRABBLE_DICTIONARY_OUT_OF_MEMORY);
            return NULL;
        }
    }

    if (ferror(file)) {
        scrabble_dictionary_destroy(dictionary);
        fclose(file);
        set_status(status, SCRABBLE_DICTIONARY_READ_FAILED);
        return NULL;
    }

    fclose(file);
    if (dictionary->count > 1) {
        qsort(dictionary->words, dictionary->count,
              sizeof(*dictionary->words), compare_words);
    }
    remove_duplicates(dictionary);
    if (!build_candidate_index(dictionary)) {
        scrabble_dictionary_destroy(dictionary);
        set_status(status, SCRABBLE_DICTIONARY_OUT_OF_MEMORY);
        return NULL;
    }
    return dictionary;
}

void scrabble_dictionary_destroy(ScrabbleDictionary *dictionary) {
    if (dictionary != NULL) {
        free(dictionary->candidate_indices);
        free(dictionary->words);
        free(dictionary);
    }
}

size_t scrabble_dictionary_count(const ScrabbleDictionary *dictionary) {
    return dictionary == NULL ? 0 : dictionary->count;
}

const char *scrabble_dictionary_word_at(
    const ScrabbleDictionary *dictionary,
    size_t index) {
    if (dictionary == NULL || index >= dictionary->count) {
        return NULL;
    }

    return dictionary->words[index];
}

int scrabble_dictionary_contains(
    const ScrabbleDictionary *dictionary,
    const char *word) {
    char normalized[SCRABBLE_MAX_WORD_LENGTH + 1];
    size_t length;

    if (dictionary == NULL || word == NULL || dictionary->count == 0) {
        return 0;
    }

    length = strlen(word);
    if (length > SCRABBLE_MAX_WORD_LENGTH) {
        return 0;
    }

    memcpy(normalized, word, length + 1);
    if (!normalize_word(normalized)) {
        return 0;
    }

    return bsearch(normalized,
                   dictionary->words,
                   dictionary->count,
                   sizeof(*dictionary->words),
                   compare_words) != NULL;
}

size_t scrabble_dictionary_candidate_count(
    const ScrabbleDictionary *dictionary,
    size_t max_length) {
    if (dictionary == NULL || max_length == 0) {
        return 0;
    }

    if (max_length > SCRABBLE_RACK_CAPACITY) {
        max_length = SCRABBLE_RACK_CAPACITY;
    }

    return dictionary->candidate_end_offsets[max_length];
}

const char *scrabble_dictionary_candidate_word_at(
    const ScrabbleDictionary *dictionary,
    size_t max_length,
    size_t index) {
    size_t candidate_count = scrabble_dictionary_candidate_count(
        dictionary, max_length);

    if (dictionary == NULL || index >= candidate_count) {
        return NULL;
    }

    return dictionary->words[dictionary->candidate_indices[index]];
}
