#include "platform/resources.h"

#include "test.h"

#include <glib/gstdio.h>

static int finds_source_resource(void) {
    GError *error = NULL;
    g_unsetenv(SCRABBLE_DATA_DIRECTORY_ENV);
    char *path = scrabble_resource_find(
        "scrabble-test-program",
        "dictionaries/demo.txt",
        &error);

    TEST_ASSERT(error == NULL);
    TEST_ASSERT(path != NULL);
    TEST_ASSERT(g_file_test(path, G_FILE_TEST_IS_REGULAR));
    g_free(path);
    return 0;
}

static int prefers_environment_override(void) {
    GError *error = NULL;
    char *temporary_directory = g_dir_make_tmp(
        "scrabble-resources-XXXXXX", &error);
    char *resource_path;
    char *found_path;

    TEST_ASSERT(error == NULL);
    TEST_ASSERT(temporary_directory != NULL);
    resource_path = g_build_filename(temporary_directory, "custom.txt", NULL);
    TEST_ASSERT(g_file_set_contents(resource_path, "WORD\n", -1, &error));
    TEST_ASSERT(error == NULL);
    TEST_ASSERT(g_setenv(
        SCRABBLE_DATA_DIRECTORY_ENV, temporary_directory, TRUE));

    found_path = scrabble_resource_find(NULL, "custom.txt", &error);
    TEST_ASSERT(error == NULL);
    TEST_ASSERT_STRING(resource_path, found_path);

    g_unsetenv(SCRABBLE_DATA_DIRECTORY_ENV);
    g_remove(resource_path);
    g_rmdir(temporary_directory);
    g_free(found_path);
    g_free(resource_path);
    g_free(temporary_directory);
    return 0;
}

static int finds_portable_share_resource(void) {
    GError *error = NULL;
    char *temporary_directory = g_dir_make_tmp(
        "scrabble-portable-XXXXXX", &error);
    char *bin_directory;
    char *data_directory;
    char *dictionary_directory;
    char *executable_path;
    char *resource_path;
    char *found_path;

    TEST_ASSERT(error == NULL);
    bin_directory = g_build_filename(temporary_directory, "bin", NULL);
    data_directory = g_build_filename(
        temporary_directory, "share", "scrabble-solver", NULL);
    dictionary_directory = g_build_filename(
        data_directory, "dictionaries", NULL);
    TEST_ASSERT(g_mkdir_with_parents(bin_directory, 0700) == 0);
    TEST_ASSERT(g_mkdir_with_parents(dictionary_directory, 0700) == 0);
    executable_path = g_build_filename(bin_directory, "solver.exe", NULL);
    resource_path = g_build_filename(dictionary_directory, "portable.txt", NULL);
    TEST_ASSERT(g_file_set_contents(resource_path, "WORD\n", -1, &error));
    TEST_ASSERT(error == NULL);

    found_path = scrabble_resource_find(
        executable_path, "dictionaries/portable.txt", &error);
    TEST_ASSERT(error == NULL);
    TEST_ASSERT(found_path != NULL);
    TEST_ASSERT(g_file_test(found_path, G_FILE_TEST_IS_REGULAR));

    g_remove(resource_path);
    g_rmdir(dictionary_directory);
    g_rmdir(data_directory);
    {
        char *share_directory = g_build_filename(
            temporary_directory, "share", NULL);
        g_rmdir(share_directory);
        g_free(share_directory);
    }
    g_rmdir(bin_directory);
    g_rmdir(temporary_directory);
    g_free(found_path);
    g_free(resource_path);
    g_free(executable_path);
    g_free(dictionary_directory);
    g_free(data_directory);
    g_free(bin_directory);
    g_free(temporary_directory);
    return 0;
}

static int reports_missing_and_invalid_resources(void) {
    GError *error = NULL;
    char *temporary_directory = g_dir_make_tmp(
        "scrabble-missing-XXXXXX", &error);
    char *path;

    TEST_ASSERT(error == NULL);
    TEST_ASSERT(temporary_directory != NULL);
    TEST_ASSERT(g_setenv(
        SCRABBLE_DATA_DIRECTORY_ENV, temporary_directory, TRUE));
    path = scrabble_resource_find(NULL, "missing.txt", &error);
    TEST_ASSERT(path == NULL);
    TEST_ASSERT(error != NULL);
    TEST_ASSERT(strstr(error->message, SCRABBLE_DATA_DIRECTORY_ENV) != NULL);
    g_clear_error(&error);
    g_unsetenv(SCRABBLE_DATA_DIRECTORY_ENV);
    g_rmdir(temporary_directory);
    g_free(temporary_directory);

    path = scrabble_resource_find(
        NULL, "missing-resource-for-test.txt", &error);

    TEST_ASSERT(path == NULL);
    TEST_ASSERT(error != NULL);
    TEST_ASSERT(strstr(error->message, SCRABBLE_DATA_DIRECTORY_ENV) != NULL);
    g_clear_error(&error);

    path = scrabble_resource_find(NULL, NULL, &error);
    TEST_ASSERT(path == NULL);
    TEST_ASSERT(error != NULL);
    g_clear_error(&error);
    return 0;
}

static const ScrabbleTestCase TESTS[] = {
    {"finds source resource", finds_source_resource},
    {"prefers environment override", prefers_environment_override},
    {"finds portable share resource", finds_portable_share_resource},
    {"reports missing and invalid resources",
     reports_missing_and_invalid_resources}
};

TEST_MAIN(TESTS)
