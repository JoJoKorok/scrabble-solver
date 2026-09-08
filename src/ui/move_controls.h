#ifndef SCRABBLE_UI_MOVE_CONTROLS_H
#define SCRABBLE_UI_MOVE_CONTROLS_H

#include "scrabble/move.h"

#include <gtk/gtk.h>

typedef enum {
    SCRABBLE_MOVE_CONTROLS_PLACE = 0,
    SCRABBLE_MOVE_CONTROLS_RECORD_OPPONENT,
    SCRABBLE_MOVE_CONTROLS_UNDO,
    SCRABBLE_MOVE_CONTROLS_NEW_GAME
} ScrabbleMoveControlsAction;

typedef struct {
    ScrabbleMoveControlsAction action;
    const char *word;
    ScrabbleBoardPosition start;
    ScrabbleMoveDirection direction;
} ScrabbleMoveControlsRequest;

/* request and request->word are valid only during the callback. */
typedef void (*ScrabbleMoveControlsCallback)(
    GtkWidget *move_controls,
    const ScrabbleMoveControlsRequest *request,
    gpointer user_data);

GtkWidget *scrabble_move_controls_new(void);

void scrabble_move_controls_set_start_position(
    GtkWidget *move_controls,
    ScrabbleBoardPosition position);
void scrabble_move_controls_set_direction(
    GtkWidget *move_controls,
    ScrabbleMoveDirection direction);
void scrabble_move_controls_set_place_available(
    GtkWidget *move_controls,
    gboolean available);
void scrabble_move_controls_set_opponent_available(
    GtkWidget *move_controls,
    gboolean available);
void scrabble_move_controls_set_history_available(
    GtkWidget *move_controls,
    gboolean available);
void scrabble_move_controls_set_word(
    GtkWidget *move_controls,
    const char *word);
/* Uses the current start and direction and the same guard as Place word.
   Returns TRUE if dispatched; the callback reports validation errors. */
gboolean scrabble_move_controls_place_word(
    GtkWidget *move_controls,
    const char *word);

/* Replaces the action callback. destroy_notify releases user_data when the
   callback is replaced or the controls are destroyed. */
void scrabble_move_controls_set_callback(
    GtkWidget *move_controls,
    ScrabbleMoveControlsCallback callback,
    gpointer user_data,
    GDestroyNotify destroy_notify);

#endif
