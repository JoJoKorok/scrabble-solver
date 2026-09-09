#include "ui/move_controls.h"

#include "scrabble/constants.h"

typedef struct {
    GtkWidget *widget;
    GtkWidget *position_label;
    GtkWidget *word_entry;
    GtkWidget *opponent_blank_entry;
    GtkWidget *direction_dropdown;
    GtkWidget *place_button;
    GtkWidget *opponent_button;
    GtkWidget *undo_button;
    GtkWidget *new_game_button;
    ScrabbleBoardPosition start_position;
    gboolean has_start_position;
    gboolean place_available;
    gboolean opponent_available;
    ScrabbleMoveControlsCallback callback;
    gpointer callback_data;
    GDestroyNotify callback_data_destroy;
} ScrabbleMoveControlsState;

static ScrabbleMoveControlsState *controls_state(GtkWidget *move_controls) {
    if (!GTK_IS_BOX(move_controls)) {
        return NULL;
    }

    return g_object_get_data(
        G_OBJECT(move_controls), "scrabble-move-controls");
}

static void update_action_buttons(ScrabbleMoveControlsState *controls) {
    const char *word = gtk_editable_get_text(
        GTK_EDITABLE(controls->word_entry));

    gtk_widget_set_sensitive(
        controls->place_button,
        controls->place_available &&
            controls->has_start_position &&
            word[0] != '\0');
    gtk_widget_set_sensitive(
        controls->opponent_button,
        controls->opponent_available &&
            controls->has_start_position &&
            word[0] != '\0');
}

static void emit_action(
    ScrabbleMoveControlsState *controls,
    ScrabbleMoveControlsAction action) {
    ScrabbleMoveControlsRequest request;

    if (controls->callback == NULL) {
        return;
    }

    request.action = action;
    request.word = gtk_editable_get_text(
        GTK_EDITABLE(controls->word_entry));
    request.opponent_blank_squares = gtk_editable_get_text(
        GTK_EDITABLE(controls->opponent_blank_entry));
    request.start = controls->start_position;
    request.direction = gtk_drop_down_get_selected(
        GTK_DROP_DOWN(controls->direction_dropdown)) == 0
            ? SCRABBLE_MOVE_HORIZONTAL
            : SCRABBLE_MOVE_VERTICAL;
    controls->callback(
        controls->widget, &request, controls->callback_data);
}

static void on_word_changed(GtkEditable *editable, gpointer user_data) {
    ScrabbleMoveControlsState *controls = user_data;

    (void)editable;
    gtk_editable_set_text(
        GTK_EDITABLE(controls->opponent_blank_entry), "");
    update_action_buttons(controls);
}

static void on_word_activated(GtkEntry *entry, gpointer user_data) {
    ScrabbleMoveControlsState *controls = user_data;

    (void)entry;
    if (gtk_widget_get_sensitive(controls->place_button)) {
        emit_action(controls, SCRABBLE_MOVE_CONTROLS_PLACE);
    }
}

static void on_place_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    emit_action(user_data, SCRABBLE_MOVE_CONTROLS_PLACE);
}

static void on_record_opponent_clicked(
    GtkButton *button,
    gpointer user_data) {
    (void)button;
    emit_action(user_data, SCRABBLE_MOVE_CONTROLS_RECORD_OPPONENT);
}

static void on_undo_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    emit_action(user_data, SCRABBLE_MOVE_CONTROLS_UNDO);
}

static void on_new_game_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    emit_action(user_data, SCRABBLE_MOVE_CONTROLS_NEW_GAME);
}

static void destroy_controls(gpointer data) {
    ScrabbleMoveControlsState *controls = data;

    if (controls->callback_data_destroy != NULL) {
        controls->callback_data_destroy(controls->callback_data);
    }
    g_free(controls);
}

GtkWidget *scrabble_move_controls_new(void) {
    static const char *directions[] = {"Horizontal", "Vertical", NULL};
    GtkWidget *controls_widget = gtk_box_new(
        GTK_ORIENTATION_VERTICAL, 10);
    ScrabbleMoveControlsState *controls = g_new0(
        ScrabbleMoveControlsState, 1);
    GtkWidget *heading = gtk_label_new("BOARD MOVE");
    GtkWidget *note = gtk_label_new(
        "The selected square is the first letter. Use Place word for your "
        "move or Record opponent move for theirs.");
    GtkWidget *opponent_blank_label = gtk_label_new(
        "Opponent blank squares (optional)");
    GtkWidget *placement_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *history_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

    controls->widget = controls_widget;
    controls->position_label = gtk_label_new("Start: select a square");
    controls->word_entry = gtk_entry_new();
    controls->opponent_blank_entry = gtk_entry_new();
    controls->direction_dropdown = gtk_drop_down_new_from_strings(directions);
    controls->place_button = gtk_button_new_with_label("Place word");
    controls->opponent_button = gtk_button_new_with_label(
        "Record opponent move");
    controls->undo_button = gtk_button_new_with_label("Undo");
    controls->new_game_button = gtk_button_new_with_label("New game");

    gtk_widget_add_css_class(controls_widget, "surface-card");
    gtk_widget_add_css_class(controls_widget, "move-controls");
    g_object_set_data_full(
        G_OBJECT(controls_widget),
        "scrabble-move-controls",
        controls,
        destroy_controls);

    gtk_widget_set_halign(heading, GTK_ALIGN_START);
    gtk_widget_add_css_class(heading, "section-label");
    gtk_box_append(GTK_BOX(controls_widget), heading);

    gtk_widget_set_halign(note, GTK_ALIGN_START);
    gtk_label_set_wrap(GTK_LABEL(note), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(note), 40);
    gtk_widget_add_css_class(note, "move-note");
    gtk_box_append(GTK_BOX(controls_widget), note);

    gtk_widget_set_halign(controls->position_label, GTK_ALIGN_START);
    gtk_widget_add_css_class(
        controls->position_label, "move-position-label");
    gtk_box_append(GTK_BOX(controls_widget), controls->position_label);

    gtk_entry_set_max_length(
        GTK_ENTRY(controls->word_entry), SCRABBLE_MAX_WORD_LENGTH);
    gtk_entry_set_placeholder_text(
        GTK_ENTRY(controls->word_entry), "Word to place");
    gtk_widget_set_hexpand(controls->word_entry, TRUE);
    gtk_widget_add_css_class(controls->word_entry, "move-word-entry");
    gtk_accessible_update_property(
        GTK_ACCESSIBLE(controls->word_entry),
        GTK_ACCESSIBLE_PROPERTY_LABEL,
        "Word to place",
        -1);
    gtk_box_append(GTK_BOX(controls_widget), controls->word_entry);

    gtk_widget_set_hexpand(controls->direction_dropdown, TRUE);
    gtk_widget_add_css_class(
        controls->direction_dropdown, "move-direction");
    gtk_accessible_update_property(
        GTK_ACCESSIBLE(controls->direction_dropdown),
        GTK_ACCESSIBLE_PROPERTY_LABEL,
        "Word direction",
        -1);
    gtk_box_append(
        GTK_BOX(placement_row), controls->direction_dropdown);

    gtk_widget_set_sensitive(controls->place_button, FALSE);
    gtk_widget_add_css_class(controls->place_button, "place-word-button");
    gtk_box_append(GTK_BOX(placement_row), controls->place_button);
    gtk_box_append(GTK_BOX(controls_widget), placement_row);

    gtk_widget_set_halign(opponent_blank_label, GTK_ALIGN_START);
    gtk_widget_add_css_class(opponent_blank_label, "move-note");
    gtk_box_append(GTK_BOX(controls_widget), opponent_blank_label);

    gtk_entry_set_max_length(GTK_ENTRY(controls->opponent_blank_entry), 16);
    gtk_entry_set_placeholder_text(
        GTK_ENTRY(controls->opponent_blank_entry),
        "Example: H9 or H9 J12");
    gtk_widget_add_css_class(
        controls->opponent_blank_entry, "opponent-blank-entry");
    gtk_accessible_update_property(
        GTK_ACCESSIBLE(controls->opponent_blank_entry),
        GTK_ACCESSIBLE_PROPERTY_LABEL,
        "Opponent blank tile squares",
        -1);
    gtk_widget_set_tooltip_text(
        controls->opponent_blank_entry,
        "Enter the board square of each blank tile the opponent placed");
    gtk_box_append(
        GTK_BOX(controls_widget), controls->opponent_blank_entry);

    gtk_widget_set_sensitive(controls->opponent_button, FALSE);
    gtk_widget_set_hexpand(controls->opponent_button, TRUE);
    gtk_widget_add_css_class(controls->opponent_button, "secondary-button");
    gtk_widget_add_css_class(
        controls->opponent_button, "opponent-move-button");
    gtk_widget_set_tooltip_text(
        controls->opponent_button,
        "Add an opponent's visible word without using your rack");
    gtk_box_append(GTK_BOX(controls_widget), controls->opponent_button);

    gtk_widget_set_sensitive(controls->undo_button, FALSE);
    gtk_widget_set_sensitive(controls->new_game_button, FALSE);
    gtk_widget_add_css_class(controls->undo_button, "secondary-button");
    gtk_widget_add_css_class(controls->new_game_button, "secondary-button");
    gtk_widget_set_hexpand(controls->undo_button, TRUE);
    gtk_widget_set_hexpand(controls->new_game_button, TRUE);
    gtk_box_append(GTK_BOX(history_row), controls->undo_button);
    gtk_box_append(GTK_BOX(history_row), controls->new_game_button);
    gtk_box_append(GTK_BOX(controls_widget), history_row);

    g_signal_connect(
        controls->word_entry,
        "changed",
        G_CALLBACK(on_word_changed),
        controls);
    g_signal_connect(
        controls->word_entry,
        "activate",
        G_CALLBACK(on_word_activated),
        controls);
    g_signal_connect(
        controls->place_button,
        "clicked",
        G_CALLBACK(on_place_clicked),
        controls);
    g_signal_connect(
        controls->opponent_button,
        "clicked",
        G_CALLBACK(on_record_opponent_clicked),
        controls);
    g_signal_connect(
        controls->undo_button,
        "clicked",
        G_CALLBACK(on_undo_clicked),
        controls);
    g_signal_connect(
        controls->new_game_button,
        "clicked",
        G_CALLBACK(on_new_game_clicked),
        controls);
    return controls_widget;
}

void scrabble_move_controls_set_start_position(
    GtkWidget *move_controls,
    ScrabbleBoardPosition position) {
    ScrabbleMoveControlsState *controls = controls_state(move_controls);
    char label[32];

    g_return_if_fail(controls != NULL);
    g_return_if_fail(scrabble_board_position_is_valid(position));
    controls->start_position = position;
    controls->has_start_position = TRUE;
    g_snprintf(
        label,
        sizeof(label),
        "Start: %c%zu",
        (char)('A' + position.column),
        position.row + 1);
    gtk_label_set_text(GTK_LABEL(controls->position_label), label);
    update_action_buttons(controls);
}

void scrabble_move_controls_set_direction(
    GtkWidget *move_controls,
    ScrabbleMoveDirection direction) {
    ScrabbleMoveControlsState *controls = controls_state(move_controls);

    g_return_if_fail(controls != NULL);
    g_return_if_fail(
        direction == SCRABBLE_MOVE_HORIZONTAL ||
        direction == SCRABBLE_MOVE_VERTICAL);
    gtk_drop_down_set_selected(
        GTK_DROP_DOWN(controls->direction_dropdown),
        direction == SCRABBLE_MOVE_HORIZONTAL ? 0 : 1);
}

void scrabble_move_controls_set_place_available(
    GtkWidget *move_controls,
    gboolean available) {
    ScrabbleMoveControlsState *controls = controls_state(move_controls);

    g_return_if_fail(controls != NULL);
    controls->place_available = available;
    update_action_buttons(controls);
}

void scrabble_move_controls_set_opponent_available(
    GtkWidget *move_controls,
    gboolean available) {
    ScrabbleMoveControlsState *controls = controls_state(move_controls);

    g_return_if_fail(controls != NULL);
    controls->opponent_available = available;
    update_action_buttons(controls);
}

void scrabble_move_controls_set_history_available(
    GtkWidget *move_controls,
    gboolean available) {
    ScrabbleMoveControlsState *controls = controls_state(move_controls);

    g_return_if_fail(controls != NULL);
    gtk_widget_set_sensitive(controls->undo_button, available);
    gtk_widget_set_sensitive(controls->new_game_button, available);
}

void scrabble_move_controls_set_word(
    GtkWidget *move_controls,
    const char *word) {
    ScrabbleMoveControlsState *controls = controls_state(move_controls);

    g_return_if_fail(controls != NULL);
    gtk_editable_set_text(
        GTK_EDITABLE(controls->word_entry), word == NULL ? "" : word);
}

void scrabble_move_controls_clear_opponent_blanks(
    GtkWidget *move_controls) {
    ScrabbleMoveControlsState *controls = controls_state(move_controls);

    g_return_if_fail(controls != NULL);
    gtk_editable_set_text(
        GTK_EDITABLE(controls->opponent_blank_entry), "");
}

gboolean scrabble_move_controls_place_word(
    GtkWidget *move_controls,
    const char *word) {
    ScrabbleMoveControlsState *controls = controls_state(move_controls);

    g_return_val_if_fail(controls != NULL, FALSE);
    if (!controls->place_available || !controls->has_start_position ||
        controls->callback == NULL || word == NULL || word[0] == '\0') {
        return FALSE;
    }

    scrabble_move_controls_set_word(move_controls, word);
    emit_action(controls, SCRABBLE_MOVE_CONTROLS_PLACE);
    return TRUE;
}

void scrabble_move_controls_set_callback(
    GtkWidget *move_controls,
    ScrabbleMoveControlsCallback callback,
    gpointer user_data,
    GDestroyNotify destroy_notify) {
    ScrabbleMoveControlsState *controls = controls_state(move_controls);

    g_return_if_fail(controls != NULL);
    if (controls->callback_data_destroy != NULL) {
        controls->callback_data_destroy(controls->callback_data);
    }
    controls->callback = callback;
    controls->callback_data = user_data;
    controls->callback_data_destroy = destroy_notify;
}
