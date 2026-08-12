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

#endif
