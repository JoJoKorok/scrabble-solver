#ifndef SCRABBLE_UI_RESULT_LIST_H
#define SCRABBLE_UI_RESULT_LIST_H

#include "scrabble/solver.h"

#include <gtk/gtk.h>

GtkWidget *scrabble_result_list_new(void);
void scrabble_result_list_clear(GtkListBox *list);
void scrabble_result_list_set_results(
    GtkListBox *list,
    const ScrabbleResultSet *results);

#endif
