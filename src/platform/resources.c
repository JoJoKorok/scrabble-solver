#include "platform/resources.h"

#include <string.h>

#ifndef SCRABBLE_SOURCE_DATA_DIR
#define SCRABBLE_SOURCE_DATA_DIR ""
#endif

#ifndef SCRABBLE_INSTALL_DATA_DIR
#define SCRABBLE_INSTALL_DATA_DIR ""
#endif

typedef enum {
    SCRABBLE_RESOURCE_NOT_FOUND = 1,
    SCRABBLE_RESOURCE_INVALID_ARGUMENT
} ScrabbleResourceError;

static GQuark scrabble_resource_error_quark(void) {
    return g_quark_from_static_string("scrabble-resource-error");
}

static char *find_executable_directory(const char *executable_path) {
    char *resolved_path;
    char *directory;

    if (executable_path == NULL || executable_path[0] == '\0') {
        return NULL;
    }

    if (g_path_is_absolute(executable_path) ||
        strchr(executable_path, '/') != NULL ||
        strchr(executable_path, '\\') != NULL) {
        resolved_path = g_canonicalize_filename(executable_path, NULL);
    } else {
        resolved_path = g_find_program_in_path(executable_path);
    }

    if (resolved_path == NULL) {
        return NULL;
    }

    directory = g_path_get_dirname(resolved_path);
    g_free(resolved_path);
    return directory;
}

static char *find_in_directory(const char *directory,
                               const char *relative_path) {
    char *candidate;

    if (directory == NULL || directory[0] == '\0') {
        return NULL;
    }

    candidate = g_build_filename(directory, relative_path, NULL);
    if (g_file_test(candidate, G_FILE_TEST_IS_REGULAR)) {
        return candidate;
    }

    g_free(candidate);
    return NULL;
}

char *scrabble_resource_find(
    const char *executable_path,
    const char *relative_path,
    GError **error) {
    const char *override_directory;
    char *executable_directory;
    char *portable_directory;
    char *candidate;

    g_return_val_if_fail(error == NULL || *error == NULL, NULL);
    if (relative_path == NULL || relative_path[0] == '\0' ||
        g_path_is_absolute(relative_path)) {
        g_set_error_literal(
            error,
            scrabble_resource_error_quark(),
            SCRABBLE_RESOURCE_INVALID_ARGUMENT,
            "The resource path must be relative.");
        return NULL;
    }

    override_directory = g_getenv(SCRABBLE_DATA_DIRECTORY_ENV);
    if (override_directory != NULL && override_directory[0] != '\0') {
        candidate = find_in_directory(override_directory, relative_path);
        if (candidate != NULL) {
            return candidate;
        }

        g_set_error(
            error,
            scrabble_resource_error_quark(),
            SCRABBLE_RESOURCE_NOT_FOUND,
            "%s is set to %s, but %s was not found there.",
            SCRABBLE_DATA_DIRECTORY_ENV,
            override_directory,
            relative_path);
        return NULL;
    }

    executable_directory = find_executable_directory(executable_path);
    portable_directory = executable_directory == NULL
        ? NULL
        : g_build_filename(executable_directory, "data", NULL);
    candidate = find_in_directory(portable_directory, relative_path);
    g_free(portable_directory);
    if (candidate != NULL) {
        g_free(executable_directory);
        return candidate;
    }

    portable_directory = executable_directory == NULL
        ? NULL
        : g_build_filename(
            executable_directory, "..", "share", "scrabble-solver", NULL);
    candidate = find_in_directory(portable_directory, relative_path);
    g_free(portable_directory);
    g_free(executable_directory);
    if (candidate != NULL) {
        return candidate;
    }

    candidate = find_in_directory(SCRABBLE_INSTALL_DATA_DIR, relative_path);
    if (candidate != NULL) {
        return candidate;
    }

    candidate = find_in_directory(SCRABBLE_SOURCE_DATA_DIR, relative_path);
    if (candidate != NULL) {
        return candidate;
    }

    g_set_error(
        error,
        scrabble_resource_error_quark(),
        SCRABBLE_RESOURCE_NOT_FOUND,
        "Could not find %s. Set %s to the application data folder.",
        relative_path,
        SCRABBLE_DATA_DIRECTORY_ENV);
    return NULL;
}
