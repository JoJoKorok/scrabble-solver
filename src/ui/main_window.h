#ifndef SCRABBLE_UI_MAIN_WINDOW_H
#define SCRABBLE_UI_MAIN_WINDOW_H

#include <gtk/gtk.h>

void scrabble_main_window_present(
    GtkApplication *application,
    const char *dictionary_path,
    const char *resource_error);

#endif
