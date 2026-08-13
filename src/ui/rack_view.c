#include "ui/rack_view.h"

#include "scrabble/constants.h"
#include "scrabble/scoring.h"

#include <string.h>

typedef struct {
    GtkWidget *tiles[SCRABBLE_RACK_CAPACITY];
    GtkWidget *letters[SCRABBLE_RACK_CAPACITY];
    GtkWidget *points[SCRABBLE_RACK_CAPACITY];
} ScrabbleRackView;

static void set_tile(
    ScrabbleRackView *view,
    size_t index,
    unsigned char character) {
    GtkWidget *tile = view->tiles[index];
    char letter_text[2] = {'\0', '\0'};
    char point_text[8] = "";
    char accessible_text[64];

    gtk_widget_remove_css_class(tile, "rack-tile-filled");
    gtk_widget_remove_css_class(tile, "rack-tile-blank");
    gtk_widget_add_css_class(tile, "rack-tile-empty");

    if (character == '\0') {
        g_snprintf(accessible_text, sizeof(accessible_text),
                   "Rack position %zu, empty", index + 1);
    } else {
        int score;

        if (character >= 'a' && character <= 'z') {
            character = (unsigned char)(character - 'a' + 'A');
        }
        letter_text[0] = (char)character;
        score = character == '?' || character == '*'
            ? 0
            : scrabble_score_word(letter_text);
        if (score >= 0) {
            g_snprintf(point_text, sizeof(point_text), "%d", score);
        }

        gtk_widget_remove_css_class(tile, "rack-tile-empty");
        gtk_widget_add_css_class(tile, "rack-tile-filled");
        if (character == '?' || character == '*') {
            letter_text[0] = '?';
            gtk_widget_add_css_class(tile, "rack-tile-blank");
            g_snprintf(accessible_text, sizeof(accessible_text),
                       "Rack position %zu, blank tile", index + 1);
        } else {
            g_snprintf(accessible_text, sizeof(accessible_text),
                       "Rack position %zu, %c, %s points",
                       index + 1, character, point_text);
        }
    }

    gtk_label_set_text(GTK_LABEL(view->letters[index]), letter_text);
    gtk_label_set_text(GTK_LABEL(view->points[index]), point_text);
    gtk_accessible_update_property(
        GTK_ACCESSIBLE(tile),
        GTK_ACCESSIBLE_PROPERTY_LABEL, accessible_text,
        -1);
}

GtkWidget *scrabble_rack_view_new(void) {
    GtkWidget *rack = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    ScrabbleRackView *view = g_new0(ScrabbleRackView, 1);

    gtk_widget_add_css_class(rack, "rack-preview");
    gtk_widget_set_halign(rack, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(rack, GTK_ALIGN_START);
    gtk_widget_set_vexpand(rack, FALSE);
    g_object_set_data_full(
        G_OBJECT(rack), "scrabble-rack-view", view, g_free);

    for (size_t index = 0; index < SCRABBLE_RACK_CAPACITY; ++index) {
        GtkWidget *tile = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        GtkWidget *letter = gtk_label_new("");
        GtkWidget *points = gtk_label_new("");

        view->tiles[index] = tile;
        view->letters[index] = letter;
        view->points[index] = points;

        gtk_widget_add_css_class(tile, "rack-tile");
        gtk_widget_add_css_class(letter, "rack-letter");
        gtk_widget_add_css_class(points, "rack-points");
        gtk_widget_set_size_request(tile, 49, 62);
        gtk_widget_set_vexpand(tile, FALSE);
        gtk_widget_set_valign(letter, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(tile), letter);
        gtk_box_append(GTK_BOX(tile), points);
        gtk_box_append(GTK_BOX(rack), tile);
        set_tile(view, index, '\0');
    }

    return rack;
}

void scrabble_rack_view_set_tiles(GtkWidget *rack_view, const char *tiles) {
    ScrabbleRackView *view;
    size_t tile_count;

    g_return_if_fail(GTK_IS_BOX(rack_view));
    view = g_object_get_data(G_OBJECT(rack_view), "scrabble-rack-view");
    g_return_if_fail(view != NULL);

    tile_count = tiles == NULL ? 0 : strlen(tiles);
    for (size_t index = 0; index < SCRABBLE_RACK_CAPACITY; ++index) {
        set_tile(view, index, index < tile_count
            ? (unsigned char)tiles[index]
            : '\0');
    }
}
