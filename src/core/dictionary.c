#include "scrabble/dictionary.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ScrabbleDictionary {
    char (*words)[SCRABBLE_MAX_WORD_LENGTH + 1];
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
                strcpy(dictionary->words[unique_count],
                       dictionary->words[index]);
            }
            ++unique_count;
        }
    }

    dictionary->count = unique_count;
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
    return dictionary;
}

void scrabble_dictionary_destroy(ScrabbleDictionary *dictionary) {
    if (dictionary != NULL) {
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
