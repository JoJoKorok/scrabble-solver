#include "platform/settings.h"

#include <errno.h>
#include <glib/gstdio.h>

enum {
    SCRABBLE_SETTINGS_INVALID_ARGUMENT = 1,
    SCRABBLE_SETTINGS_CREATE_DIRECTORY_FAILED
};

static const char *SETTINGS_GROUP = "Dictionary";
static const char *DICTIONARY_PATH_KEY = "path";

static GQuark scrabble_settings_error_quark(void) {
    return g_quark_from_static_string("scrabble-settings-error");
}

static gboolean validate_settings_path(
    const char *settings_path,
    GError **error) {
    g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

    if (settings_path == NULL || settings_path[0] == '\0') {
        g_set_error_literal(
            error,
            scrabble_settings_error_quark(),
            SCRABBLE_SETTINGS_INVALID_ARGUMENT,
            "The settings path is required.");
        return FALSE;
    }

    return TRUE;
}

static gboolean load_existing_key_file(
    GKeyFile *key_file,
    const char *settings_path,
    GError **error) {
    GError *load_error = NULL;

    if (g_key_file_load_from_file(
            key_file, settings_path, G_KEY_FILE_KEEP_COMMENTS, &load_error)) {
        return TRUE;
    }

    if (g_error_matches(load_error, G_FILE_ERROR, G_FILE_ERROR_NOENT)) {
        g_clear_error(&load_error);
        return TRUE;
    }

    g_propagate_error(error, load_error);
    return FALSE;
}

static gboolean ensure_settings_directory(
    const char *settings_path,
    GError **error) {
    char *directory = g_path_get_dirname(settings_path);
    int result = g_mkdir_with_parents(directory, 0700);

    if (result != 0) {
        g_set_error(
            error,
            scrabble_settings_error_quark(),
            SCRABBLE_SETTINGS_CREATE_DIRECTORY_FAILED,
            "Could not create settings directory %s: %s",
            directory,
            g_strerror(errno));
    }

    g_free(directory);
    return result == 0;
}

char *scrabble_settings_default_path(void) {
    return g_build_filename(
        g_get_user_config_dir(), "scrabble-solver", "settings.ini", NULL);
}

char *scrabble_settings_load_dictionary_path(
    const char *settings_path,
    GError **error) {
    GKeyFile *key_file;
    GError *value_error = NULL;
    char *dictionary_path;

    if (!validate_settings_path(settings_path, error)) {
        return NULL;
    }

    key_file = g_key_file_new();
    if (!load_existing_key_file(key_file, settings_path, error)) {
        g_key_file_free(key_file);
        return NULL;
    }

    dictionary_path = g_key_file_get_string(
        key_file, SETTINGS_GROUP, DICTIONARY_PATH_KEY, &value_error);
    g_key_file_free(key_file);
    if (value_error != NULL) {
        if (g_error_matches(
                value_error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND) ||
            g_error_matches(
                value_error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND)) {
            g_clear_error(&value_error);
            return NULL;
        }

        g_propagate_error(error, value_error);
        return NULL;
    }

    if (dictionary_path[0] == '\0') {
        g_free(dictionary_path);
        return NULL;
    }

    return dictionary_path;
}

gboolean scrabble_settings_save_dictionary_path(
    const char *settings_path,
    const char *dictionary_path,
    GError **error) {
    GKeyFile *key_file;
    gboolean saved;

    g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
    if (!validate_settings_path(settings_path, error)) {
        return FALSE;
    }
    if (dictionary_path == NULL || dictionary_path[0] == '\0') {
        g_set_error_literal(
            error,
            scrabble_settings_error_quark(),
            SCRABBLE_SETTINGS_INVALID_ARGUMENT,
            "The dictionary path is required.");
        return FALSE;
    }

    key_file = g_key_file_new();
    if (!load_existing_key_file(key_file, settings_path, error)) {
        g_key_file_free(key_file);
        return FALSE;
    }

    g_key_file_set_string(
        key_file, SETTINGS_GROUP, DICTIONARY_PATH_KEY, dictionary_path);
    saved = ensure_settings_directory(settings_path, error) &&
        g_key_file_save_to_file(key_file, settings_path, error);
    g_key_file_free(key_file);
    return saved;
}

gboolean scrabble_settings_clear_dictionary_path(
    const char *settings_path,
    GError **error) {
    GKeyFile *key_file;
    GError *remove_error = NULL;
    gboolean saved;

    if (!validate_settings_path(settings_path, error)) {
        return FALSE;
    }

    key_file = g_key_file_new();
    if (!load_existing_key_file(key_file, settings_path, error)) {
        g_key_file_free(key_file);
        return FALSE;
    }

    if (!g_key_file_remove_key(
            key_file, SETTINGS_GROUP, DICTIONARY_PATH_KEY, &remove_error)) {
        if (g_error_matches(
                remove_error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND) ||
            g_error_matches(
                remove_error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND)) {
            g_clear_error(&remove_error);
            g_key_file_free(key_file);
            return TRUE;
        }

        g_propagate_error(error, remove_error);
        g_key_file_free(key_file);
        return FALSE;
    }

    saved = ensure_settings_directory(settings_path, error) &&
        g_key_file_save_to_file(key_file, settings_path, error);
    g_key_file_free(key_file);
    return saved;
}
