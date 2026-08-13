#ifndef SCRABBLE_UI_THEME_H
#define SCRABBLE_UI_THEME_H

#include <gtk/gtk.h>

/* Applies the application stylesheet to the default display. */
int scrabble_theme_apply(const char *stylesheet_path, GError **error);

#endif
