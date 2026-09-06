#include "ui/result_list.h"

typedef struct {
    gboolean placement_available;
    ScrabbleResultListCallback callback;
    gpointer callback_data;
    GDestroyNotify callback_data_destroy;
} ScrabbleResultListState;

typedef struct {
    ScrabbleResult result;
    GtkWidget *place_button;
} ScrabbleResultRow;

static void destroy_list_state(gpointer data) {
    ScrabbleResultListState *state = data;

    if (state->callback_data_destroy != NULL) {
        state->callback_data_destroy(state->callback_data);
    }
    g_free(state);
}

static void update_row(GtkListBoxRow *row, ScrabbleResultListState *state) {
    ScrabbleResultRow *data = g_object_get_data(G_OBJECT(row), "scrabble-result");
    gboolean available = state->placement_available &&
        state->callback != NULL && data->result.word[1] != '\0';

    gtk_widget_set_sensitive(data->place_button, available);
    gtk_list_box_row_set_activatable(row, available);
}

static void on_row_activated(
    GtkListBox *list,
    GtkListBoxRow *row,
    gpointer user_data) {
    ScrabbleResultListState *state = user_data;
    ScrabbleResultRow *data = g_object_get_data(G_OBJECT(row), "scrabble-result");
    ScrabbleResult result;

    if (!state->placement_available || state->callback == NULL ||
        data == NULL || data->result.word[1] == '\0' ||
        gtk_widget_get_parent(GTK_WIDGET(row)) != GTK_WIDGET(list)) {
        return;
    }

    /* A callback may clear or rebuild the list while using this result. */
    result = data->result;
    state->callback(list, &result, state->callback_data);
}

static void on_place_clicked(GtkButton *button, gpointer user_data) {
    GtkListBoxRow *row = GTK_LIST_BOX_ROW(user_data);
    GtkWidget *list = gtk_widget_get_parent(GTK_WIDGET(row));
    ScrabbleResultListState *state;

    (void)button;
    if (!GTK_IS_LIST_BOX(list)) {
        return;
    }
    state = g_object_get_data(G_OBJECT(list), "scrabble-result-list");
    on_row_activated(GTK_LIST_BOX(list), row, state);
}

static GtkWidget *create_result_row(
    const ScrabbleResult *result,
    ScrabbleResultListState *state) {
    GtkWidget *list_row = gtk_list_box_row_new();
    ScrabbleResultRow *data = g_new0(ScrabbleResultRow, 1);
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget *word = gtk_label_new(result->word);
    char score_text[32];
    GtkWidget *score;
    char accessible_text[96];

    data->result = *result;
    data->place_button = gtk_button_new_with_label("Place");
    g_object_set_data_full(
        G_OBJECT(list_row), "scrabble-result", data, g_free);
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(list_row), row);

    g_snprintf(
        score_text,
        sizeof(score_text),
        "%d %s",
        result->score,
        result->score == 1 ? "point" : "points");
    score = gtk_label_new(score_text);

    gtk_widget_set_margin_top(row, 8);
    gtk_widget_set_margin_bottom(row, 8);
    gtk_widget_set_margin_start(row, 12);
    gtk_widget_set_margin_end(row, 12);
    gtk_widget_add_css_class(row, "result-row");

    gtk_widget_set_halign(word, GTK_ALIGN_START);
    gtk_widget_set_hexpand(word, TRUE);
    gtk_widget_add_css_class(word, "result-word");
    gtk_box_append(GTK_BOX(row), word);

    if (result->has_placement) {
        char placement_text[32];
        GtkWidget *placement;

        g_snprintf(
            placement_text,
            sizeof(placement_text),
            "%c%zu %s",
            (char)('A' + result->move.start.column),
            result->move.start.row + 1,
            result->move.direction == SCRABBLE_MOVE_HORIZONTAL
                ? "across"
                : "down");
        placement = gtk_label_new(placement_text);
        gtk_widget_add_css_class(placement, "placement-chip");
        gtk_box_append(GTK_BOX(row), placement);
    }

    if (result->rack_match.blanks_used > 0) {
        char blank_text[32];
        GtkWidget *blank_note;

        g_snprintf(
            blank_text,
            sizeof(blank_text),
            "%zu %s",
            result->rack_match.blanks_used,
            result->rack_match.blanks_used == 1 ? "blank" : "blanks");
        blank_note = gtk_label_new(blank_text);
        gtk_widget_add_css_class(blank_note, "blank-chip");
        gtk_box_append(GTK_BOX(row), blank_note);
    }

    gtk_widget_set_halign(score, GTK_ALIGN_END);
    gtk_widget_add_css_class(score, "score-chip");
    gtk_box_append(GTK_BOX(row), score);
    gtk_widget_add_css_class(data->place_button, "secondary-button");
    gtk_widget_add_css_class(data->place_button, "result-place-button");
    if (result->has_placement) {
        g_snprintf(
            accessible_text,
            sizeof(accessible_text),
            "Place %s at %c%zu %s",
            result->word,
            (char)('A' + result->move.start.column),
            result->move.start.row + 1,
            result->move.direction == SCRABBLE_MOVE_HORIZONTAL
                ? "across"
                : "down");
    } else {
        g_snprintf(
            accessible_text,
            sizeof(accessible_text),
            "Place %s at the selected square and direction",
            result->word);
    }
    gtk_accessible_update_property(
        GTK_ACCESSIBLE(data->place_button),
        GTK_ACCESSIBLE_PROPERTY_LABEL, accessible_text, -1);
    gtk_widget_set_tooltip_text(data->place_button,
        result->word[1] == '\0'
            ? "An opening word needs at least two letters."
            : result->has_placement
                ? "Place this legal move at its suggested board position."
                : "Place on an empty board using the selected square and direction.");
    gtk_box_append(GTK_BOX(row), data->place_button);
    g_signal_connect(data->place_button, "clicked",
                     G_CALLBACK(on_place_clicked), list_row);
    update_row(GTK_LIST_BOX_ROW(list_row), state);
    return list_row;
}

GtkWidget *scrabble_result_list_new(void) {
    GtkWidget *list = gtk_list_box_new();
    ScrabbleResultListState *state = g_new0(ScrabbleResultListState, 1);

    g_object_set_data_full(
        G_OBJECT(list), "scrabble-result-list", state, destroy_list_state);
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list), GTK_SELECTION_SINGLE);
    gtk_list_box_set_activate_on_single_click(GTK_LIST_BOX(list), FALSE);
    g_signal_connect(list, "row-activated", G_CALLBACK(on_row_activated), state);
    gtk_widget_add_css_class(list, "results-list");
    return list;
}

void scrabble_result_list_set_placement_available(
    GtkListBox *list,
    gboolean available) {
    ScrabbleResultListState *state;

    g_return_if_fail(GTK_IS_LIST_BOX(list));
    state = g_object_get_data(G_OBJECT(list), "scrabble-result-list");
    g_return_if_fail(state != NULL);
    state->placement_available = available;
    for (GtkWidget *row = gtk_widget_get_first_child(GTK_WIDGET(list));
         row != NULL;
         row = gtk_widget_get_next_sibling(row)) {
        update_row(GTK_LIST_BOX_ROW(row), state);
    }
}

void scrabble_result_list_set_callback(
    GtkListBox *list,
    ScrabbleResultListCallback callback,
    gpointer user_data,
    GDestroyNotify destroy_notify) {
    ScrabbleResultListState *state;

    g_return_if_fail(GTK_IS_LIST_BOX(list));
    state = g_object_get_data(G_OBJECT(list), "scrabble-result-list");
    g_return_if_fail(state != NULL);
    if (state->callback_data_destroy != NULL) {
        state->callback_data_destroy(state->callback_data);
    }
    state->callback = callback;
    state->callback_data = user_data;
    state->callback_data_destroy = destroy_notify;
    scrabble_result_list_set_placement_available(list, state->placement_available);
}

void scrabble_result_list_clear(GtkListBox *list) {
    GtkWidget *child;

    g_return_if_fail(GTK_IS_LIST_BOX(list));
    while ((child = gtk_widget_get_first_child(GTK_WIDGET(list))) != NULL) {
        gtk_list_box_remove(list, child);
    }
}

void scrabble_result_list_set_results(
    GtkListBox *list,
    const ScrabbleResultSet *results) {
    ScrabbleResultListState *state;

    g_return_if_fail(GTK_IS_LIST_BOX(list));
    g_return_if_fail(results != NULL);
    state = g_object_get_data(G_OBJECT(list), "scrabble-result-list");
    g_return_if_fail(state != NULL);

    scrabble_result_list_clear(list);
    for (size_t index = 0; index < results->count; ++index) {
        gtk_list_box_append(list, create_result_row(&results->items[index], state));
    }
}
