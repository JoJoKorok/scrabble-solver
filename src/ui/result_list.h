#ifndef SCRABBLE_UI_RESULT_LIST_H
#define SCRABBLE_UI_RESULT_LIST_H

#include "scrabble/solver.h"

#include <gtk/gtk.h>

/* The result is a temporary copy valid for the duration of the callback. */
typedef void (*ScrabbleResultListCallback)(
    GtkListBox *list,
    const ScrabbleResult *result,
    gpointer user_data);

GtkWidget *scrabble_result_list_new(void);
/* Placement starts disabled. Disabling it keeps rack results readable. */
void scrabble_result_list_set_placement_available(
    GtkListBox *list,
    gboolean available);
/* Replaces the callback; destroy_notify owns the callback data's cleanup. */
void scrabble_result_list_set_callback(
    GtkListBox *list,
    ScrabbleResultListCallback callback,
    gpointer user_data,
    GDestroyNotify destroy_notify);
void scrabble_result_list_clear(GtkListBox *list);
/* Rows own copies so results may be destroyed immediately after this call. */
void scrabble_result_list_set_results(
    GtkListBox *list,
    const ScrabbleResultSet *results);

#endif
