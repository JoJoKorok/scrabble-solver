#ifndef SCRABBLE_UI_BOARD_VIEW_H
#define SCRABBLE_UI_BOARD_VIEW_H

#include "scrabble/board.h"

#include <gtk/gtk.h>

typedef void (*ScrabbleBoardViewSelectionCallback)(
    GtkWidget *board_view,
    ScrabbleBoardPosition position,
    gpointer user_data);

GtkWidget *scrabble_board_view_new(void);

/* Copies the board's current cells into the view. Passing NULL displays an
   empty board. The caller retains ownership of board. */
void scrabble_board_view_set_board(
    GtkWidget *board_view,
    const ScrabbleBoard *board);

/* Programmatic selection does not invoke the selection callback. */
void scrabble_board_view_select_position(
    GtkWidget *board_view,
    ScrabbleBoardPosition position);
gboolean scrabble_board_view_get_selected_position(
    GtkWidget *board_view,
    ScrabbleBoardPosition *position);
void scrabble_board_view_clear_selection(GtkWidget *board_view);

/* Replaces the selection callback. destroy_notify releases user_data when the
   callback is replaced or the view is destroyed. */
void scrabble_board_view_set_selection_callback(
    GtkWidget *board_view,
    ScrabbleBoardViewSelectionCallback callback,
    gpointer user_data,
    GDestroyNotify destroy_notify);

#endif
