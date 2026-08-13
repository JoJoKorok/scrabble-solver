#ifndef SCRABBLE_UI_RACK_VIEW_H
#define SCRABBLE_UI_RACK_VIEW_H

#include <gtk/gtk.h>

GtkWidget *scrabble_rack_view_new(void);
void scrabble_rack_view_set_tiles(GtkWidget *rack_view, const char *tiles);

#endif
