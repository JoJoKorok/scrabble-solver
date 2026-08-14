#ifndef SCRABBLE_PLATFORM_SETTINGS_H
#define SCRABBLE_PLATFORM_SETTINGS_H

#include <glib.h>

/* Returns the platform-appropriate settings file path. The caller owns it. */
char *scrabble_settings_default_path(void);

/* Returns the saved custom dictionary path, or NULL when none is saved.
   The caller owns the returned path. */
char *scrabble_settings_load_dictionary_path(
    const char *settings_path,
    GError **error);

gboolean scrabble_settings_save_dictionary_path(
    const char *settings_path,
    const char *dictionary_path,
    GError **error);

gboolean scrabble_settings_clear_dictionary_path(
    const char *settings_path,
    GError **error);

#endif
