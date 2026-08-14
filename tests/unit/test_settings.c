#include "platform/settings.h"

#include "test.h"

#include <glib/gstdio.h>

static int saves_loads_and_clears_dictionary_path(void) {
    GError *error = NULL;
    char *temporary_directory = g_dir_make_tmp(
        "scrabble-settings-XXXXXX", &error);
    char *settings_directory;
    char *settings_path;
    char *loaded_path;

    TEST_ASSERT(error == NULL);
    TEST_ASSERT(temporary_directory != NULL);
    settings_directory = g_build_filename(
        temporary_directory, "configuration", NULL);
    settings_path = g_build_filename(
        settings_directory, "settings.ini", NULL);

    loaded_path = scrabble_settings_load_dictionary_path(
        settings_path, &error);
    TEST_ASSERT(error == NULL);
    TEST_ASSERT(loaded_path == NULL);

    TEST_ASSERT(scrabble_settings_save_dictionary_path(
        settings_path, "C:/Word Lists/custom.txt", &error));
    TEST_ASSERT(error == NULL);
    TEST_ASSERT(g_file_test(settings_path, G_FILE_TEST_IS_REGULAR));

    loaded_path = scrabble_settings_load_dictionary_path(
        settings_path, &error);
    TEST_ASSERT(error == NULL);
    TEST_ASSERT_STRING("C:/Word Lists/custom.txt", loaded_path);
    g_free(loaded_path);

    TEST_ASSERT(scrabble_settings_save_dictionary_path(
        settings_path, "/tmp/replacement.lst", &error));
    TEST_ASSERT(error == NULL);
    loaded_path = scrabble_settings_load_dictionary_path(
        settings_path, &error);
    TEST_ASSERT_STRING("/tmp/replacement.lst", loaded_path);
    g_free(loaded_path);

    TEST_ASSERT(scrabble_settings_clear_dictionary_path(
        settings_path, &error));
    TEST_ASSERT(error == NULL);
    loaded_path = scrabble_settings_load_dictionary_path(
        settings_path, &error);
    TEST_ASSERT(error == NULL);
    TEST_ASSERT(loaded_path == NULL);

    g_remove(settings_path);
    g_rmdir(settings_directory);
    g_rmdir(temporary_directory);
    g_free(settings_path);
    g_free(settings_directory);
    g_free(temporary_directory);
    return 0;
}

static int reports_invalid_and_malformed_settings(void) {
    GError *error = NULL;
    char *temporary_directory = g_dir_make_tmp(
        "scrabble-bad-settings-XXXXXX", &error);
    char *settings_path;
    char *loaded_path;

    TEST_ASSERT(error == NULL);
    TEST_ASSERT(temporary_directory != NULL);
    settings_path = g_build_filename(
        temporary_directory, "settings.ini", NULL);

    TEST_ASSERT(!scrabble_settings_save_dictionary_path(
        settings_path, NULL, &error));
    TEST_ASSERT(error != NULL);
    g_clear_error(&error);

    TEST_ASSERT(g_file_set_contents(
        settings_path, "not valid key file data", -1, &error));
    TEST_ASSERT(error == NULL);
    loaded_path = scrabble_settings_load_dictionary_path(
        settings_path, &error);
    TEST_ASSERT(loaded_path == NULL);
    TEST_ASSERT(error != NULL);
    g_clear_error(&error);

    g_remove(settings_path);
    g_rmdir(temporary_directory);
    g_free(settings_path);
    g_free(temporary_directory);
    return 0;
}

static const ScrabbleTestCase TESTS[] = {
    {"saves, loads, and clears dictionary path",
     saves_loads_and_clears_dictionary_path},
    {"reports invalid and malformed settings",
     reports_invalid_and_malformed_settings}
};

TEST_MAIN(TESTS)
