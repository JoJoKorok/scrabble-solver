#include "ui/result_list.h"

static GtkWidget *create_result_row(const ScrabbleResult *result) {
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget *word = gtk_label_new(result->word);
    char score_text[32];
    GtkWidget *score;

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
    return row;
}

GtkWidget *scrabble_result_list_new(void) {
    GtkWidget *list = gtk_list_box_new();

    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list), GTK_SELECTION_NONE);
    gtk_widget_add_css_class(list, "results-list");
    return list;
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
    g_return_if_fail(GTK_IS_LIST_BOX(list));
    g_return_if_fail(results != NULL);

    scrabble_result_list_clear(list);
    for (size_t index = 0; index < results->count; ++index) {
        gtk_list_box_append(list, create_result_row(&results->items[index]));
    }
}
