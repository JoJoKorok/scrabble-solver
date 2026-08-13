#ifndef SCRABBLE_PLATFORM_RESOURCES_H
#define SCRABBLE_PLATFORM_RESOURCES_H

#include <glib.h>

#define SCRABBLE_DATA_DIRECTORY_ENV "SCRABBLE_SOLVER_DATA_DIR"

/* Finds an application data file using, in order: an environment override,
   portable locations beside the executable, the install location, and the
   source data directory. The caller owns the returned path. */
char *scrabble_resource_find(
    const char *executable_path,
    const char *relative_path,
    GError **error);

/* Finds a bundled resource without consulting the environment override.
   This is useful for optional presentation assets that should fall back to
   the application installation even when custom data is incomplete. */
char *scrabble_resource_find_bundled(
    const char *executable_path,
    const char *relative_path,
    GError **error);

#endif
