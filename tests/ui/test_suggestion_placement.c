#include "platform/settings.h"
#include "platform/resources.h"
#include "ui/main_window.h"
#include "ui/result_list.h"
#include "ui/theme.h"

#include "test_paths.h"

#include <string.h>

typedef struct {
    GtkApplication *application;
    GtkWidget *window;
    GtkWidget *rack;
    GtkWidget *board;
    GtkWidget *controls;
    GtkWidget *status;
    GtkListBox *results;
} WindowFixture;

static GtkWidget *find_class(GtkWidget *root, const char *css_class) {
    if (gtk_widget_has_css_class(root, css_class)) {
        return root;
    }
    for (GtkWidget *child = gtk_widget_get_first_child(root);
         child != NULL;
         child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = find_class(child, css_class);
        if (found != NULL) {
            return found;
        }
    }
    return NULL;
}

static GtkWidget *find_button(GtkWidget *root, const char *label) {
    if (GTK_IS_BUTTON(root) &&
        g_strcmp0(gtk_button_get_label(GTK_BUTTON(root)), label) == 0) {
        return root;
    }
    for (GtkWidget *child = gtk_widget_get_first_child(root);
         child != NULL;
         child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = find_button(child, label);
        if (found != NULL) {
            return found;
        }
    }
    return NULL;
}

static void click(GtkWidget *button) {
    g_assert_nonnull(button);
    g_assert_true(gtk_widget_is_sensitive(button));
    g_signal_emit_by_name(button, "clicked");
}

static GtkWidget *square(WindowFixture *fixture, size_t row, size_t column) {
    GtkWidget *cell = gtk_grid_get_child_at(
        GTK_GRID(fixture->board), (int)column + 1, (int)row + 1);
    g_assert_nonnull(cell);
    return cell;
}

static GtkListBoxRow *result_row(GtkListBox *list, const char *word) {
    for (GtkWidget *row = gtk_widget_get_first_child(GTK_WIDGET(list));
         row != NULL;
         row = gtk_widget_get_next_sibling(row)) {
        GtkWidget *label = find_class(row, "result-word");
        if (g_strcmp0(gtk_label_get_text(GTK_LABEL(label)), word) == 0) {
            return GTK_LIST_BOX_ROW(row);
        }
    }
    g_error("Missing result: %s", word);
    return NULL;
}

static GtkListBoxRow *result_row_at(
    GtkListBox *list,
    const char *word,
    const char *placement) {
    for (GtkWidget *row = gtk_widget_get_first_child(GTK_WIDGET(list));
         row != NULL;
         row = gtk_widget_get_next_sibling(row)) {
        GtkWidget *word_label = find_class(row, "result-word");
        GtkWidget *placement_label = find_class(row, "placement-chip");

        if (word_label != NULL && placement_label != NULL &&
            g_strcmp0(
                gtk_label_get_text(GTK_LABEL(word_label)), word) == 0 &&
            g_strcmp0(
                gtk_label_get_text(GTK_LABEL(placement_label)),
                placement) == 0) {
            return GTK_LIST_BOX_ROW(row);
        }
    }
    g_error("Missing result: %s at %s", word, placement);
    return NULL;
}

static void setup_window(WindowFixture *fixture, gconstpointer data) {
    GError *error = NULL;
    char *settings_path = scrabble_settings_default_path();

    (void)data;
    /* g_test_init(ISOLATE_DIRS) directs this into the test's temporary tree. */
    g_assert_true(scrabble_settings_save_dictionary_path(
        settings_path, SCRABBLE_SOLVER_DICTIONARY_FIXTURE, &error));
    g_assert_no_error(error);
    g_free(settings_path);

    g_test_message("Registering test application");
    fixture->application = gtk_application_new(
        "com.jojokorok.scrabblesolver.tests", G_APPLICATION_NON_UNIQUE);
    g_assert_true(g_application_register(
        G_APPLICATION(fixture->application), NULL, &error));
    g_assert_no_error(error);
    {
        char *stylesheet = scrabble_resource_find_bundled(
            NULL, "styles/application.css", &error);
        g_assert_no_error(error);
        g_assert_true(scrabble_theme_apply(stylesheet, &error));
        g_assert_no_error(error);
        g_free(stylesheet);
    }
    g_test_message("Creating main window");
    scrabble_main_window_present(
        fixture->application, SCRABBLE_SOLVER_DICTIONARY_FIXTURE, NULL);
    g_test_message("Finding workflow controls");
    fixture->window = GTK_WIDGET(gtk_application_get_active_window(
        fixture->application));
    g_assert_nonnull(fixture->window);
    fixture->rack = find_class(fixture->window, "rack-entry");
    fixture->board = find_class(fixture->window, "scrabble-board");
    fixture->controls = find_class(fixture->window, "move-controls");
    fixture->status = find_class(fixture->window, "status-label");
    fixture->results = GTK_LIST_BOX(find_class(fixture->window, "results-list"));
    g_assert_nonnull(fixture->rack);
    g_assert_nonnull(fixture->board);
    g_assert_nonnull(fixture->controls);
    g_assert_nonnull(fixture->status);
    g_assert_nonnull(fixture->results);
}

static void teardown_window(WindowFixture *fixture, gconstpointer data) {
    (void)data;
    gtk_window_destroy(GTK_WINDOW(fixture->window));
    g_object_unref(fixture->application);
}

static void search(WindowFixture *fixture, const char *rack) {
    gtk_editable_set_text(GTK_EDITABLE(fixture->rack), rack);
    click(find_class(fixture->window, "solve-button"));
}

static void assert_word(
    WindowFixture *fixture,
    const char *word,
    size_t row,
    size_t column,
    gboolean vertical) {
    for (size_t i = 0; word[i] != '\0'; ++i) {
        GtkWidget *cell = square(
            fixture, row + (vertical ? i : 0), column + (vertical ? 0 : i));
        char letter[] = {word[i], '\0'};
        g_assert_true(gtk_widget_has_css_class(cell, "board-tile-filled"));
        g_assert_cmpstr(gtk_label_get_text(GTK_LABEL(find_class(
            cell, "board-cell-primary"))), ==, letter);
    }
}

static void assert_board_empty(WindowFixture *fixture) {
    for (size_t row = 0; row < SCRABBLE_BOARD_SIZE; ++row) {
        for (size_t column = 0; column < SCRABBLE_BOARD_SIZE; ++column) {
            g_assert_false(gtk_widget_has_css_class(
                square(fixture, row, column), "board-tile-filled"));
        }
    }
}

/* Optional visual artifact, rendered by GTK without desktop input automation. */
static void save_window_snapshot(WindowFixture *fixture) {
    const char *path = g_getenv("SCRABBLE_TEST_SCREENSHOT");
    GdkPaintable *paintable;
    GtkSnapshot *snapshot;
    GskRenderNode *node;
    GdkTexture *texture;

    if (path == NULL) {
        return;
    }
    /* Let layout settle, then show the suggestions at the bottom of the pane. */
    for (int frame = 0; frame < 30; ++frame) {
        while (g_main_context_iteration(NULL, FALSE)) {
        }
        g_usleep(10000);
    }
    {
        GtkAdjustment *adjustment = gtk_scrolled_window_get_vadjustment(
            GTK_SCROLLED_WINDOW(find_class(fixture->window, "workflow-scroll")));
        gtk_adjustment_set_value(adjustment, gtk_adjustment_get_upper(adjustment));
        for (int frame = 0; frame < 10; ++frame) {
            while (g_main_context_iteration(NULL, FALSE)) {
            }
            g_usleep(10000);
        }
    }
    paintable = gtk_widget_paintable_new(fixture->window);
    snapshot = gtk_snapshot_new();
    gdk_paintable_snapshot(
        paintable, GDK_SNAPSHOT(snapshot),
        gtk_widget_get_width(fixture->window),
        gtk_widget_get_height(fixture->window));
    node = gtk_snapshot_free_to_node(snapshot);
    g_assert_nonnull(node);
    texture = gsk_renderer_render_texture(
        gtk_native_get_renderer(GTK_NATIVE(fixture->window)), node, NULL);
    g_assert_nonnull(texture);
    g_assert_true(gdk_texture_save_to_png(texture, path));
    g_object_unref(texture);
    gsk_render_node_unref(node);
    g_object_unref(paintable);
}

static void places_opening_and_connected_suggestions(
    WindowFixture *fixture,
    gconstpointer data) {
    GtkListBoxRow *row;
    (void)data;

    g_test_message("Searching and placing the horizontal opening");
    search(fixture, "RET?INS");
    row = result_row(fixture->results, "RETAINS");
    /* The solver's result set has already been destroyed by search(). */
    g_assert_cmpstr(gtk_label_get_text(GTK_LABEL(find_class(
        GTK_WIDGET(row), "score-chip"))), ==, "6 points");
    click(square(fixture, 7, 4));
    click(find_button(GTK_WIDGET(row), "Place"));
    assert_word(fixture, "RETAINS", 7, 4, FALSE);
    g_assert_true(gtk_widget_has_css_class(
        square(fixture, 7, 7), "board-blank-tile"));
    g_assert_nonnull(strstr(
        gtk_label_get_text(GTK_LABEL(fixture->status)), "62 points"));
    g_assert_null(gtk_widget_get_first_child(GTK_WIDGET(fixture->results)));

    g_test_message("Searching and placing a connected board suggestion");
    search(fixture, "RIN");
    row = result_row_at(fixture->results, "RAIN", "H7 down");
    g_assert_cmpstr(gtk_label_get_text(GTK_LABEL(find_class(
        GTK_WIDGET(row), "score-chip"))), ==, "3 points");
    click(find_button(GTK_WIDGET(row), "Place"));
    assert_word(fixture, "RAIN", 6, 7, TRUE);
    g_assert_cmpuint(gtk_drop_down_get_selected(GTK_DROP_DOWN(find_class(
        fixture->controls, "move-direction"))), ==, 1);
    g_assert_nonnull(strstr(
        gtk_label_get_text(GTK_LABEL(fixture->status)), "3 points"));
    g_assert_null(gtk_widget_get_first_child(GTK_WIDGET(fixture->results)));
    save_window_snapshot(fixture);

    g_test_message("Undoing the connected move without disturbing the opening");
    click(find_button(fixture->controls, "Undo"));
    g_assert_false(gtk_widget_has_css_class(
        square(fixture, 6, 7), "board-tile-filled"));
    assert_word(fixture, "RETAINS", 7, 4, FALSE);

    g_test_message("Replaying a move, then starting a clean new game");
    search(fixture, "RIN");
    row = result_row_at(fixture->results, "RAIN", "H7 down");
    click(find_button(GTK_WIDGET(row), "Place"));
    search(fixture, "TRAIN");
    g_assert_nonnull(gtk_widget_get_first_child(GTK_WIDGET(fixture->results)));
    click(find_button(fixture->controls, "New game"));
    assert_board_empty(fixture);
    g_assert_null(gtk_widget_get_first_child(GTK_WIDGET(fixture->results)));
    g_assert_false(gtk_widget_get_sensitive(
        find_button(fixture->controls, "Undo")));
    g_assert_cmpuint(gtk_drop_down_get_selected(GTK_DROP_DOWN(find_class(
        fixture->controls, "move-direction"))), ==, 0);
    g_assert_cmpstr(gtk_editable_get_text(GTK_EDITABLE(find_class(
        fixture->controls, "move-word-entry"))), ==, "");
    g_assert_true(gtk_widget_has_css_class(
        square(fixture, 7, 7), "board-cell-selected"));
    g_assert_nonnull(strstr(
        gtk_label_get_text(GTK_LABEL(fixture->status)), "opening words"));

    g_test_message("Placing a fresh opening after reset");
    search(fixture, "RETAINS");
    row = result_row(fixture->results, "RETAINS");
    click(find_button(GTK_WIDGET(row), "Place"));
    assert_word(fixture, "RETAINS", 7, 7, FALSE);
    g_assert_nonnull(strstr(
        gtk_label_get_text(GTK_LABEL(fixture->status)), "66 points"));
    click(find_button(fixture->controls, "Undo"));
    assert_board_empty(fixture);
}

static void rejects_invalid_positions_and_revalidates_rack(
    WindowFixture *fixture,
    gconstpointer data) {
    GtkListBoxRow *row;
    ScrabbleResult stale = {0};
    ScrabbleResultSet stale_results = {&stale, 1};
    (void)data;

    search(fixture, "RETAINS");
    row = result_row(fixture->results, "RETAINS");
    click(square(fixture, 0, 0));
    click(find_button(GTK_WIDGET(row), "Place"));
    g_assert_nonnull(strstr(
        gtk_label_get_text(GTK_LABEL(fixture->status)), "center star"));
    g_assert_false(gtk_widget_has_css_class(
        square(fixture, 0, 0), "board-tile-filled"));
    click(square(fixture, 7, 14));
    click(find_button(GTK_WIDGET(row), "Place"));
    g_assert_nonnull(strstr(
        gtk_label_get_text(GTK_LABEL(fixture->status)), "edge of the board"));
    g_assert_false(gtk_widget_get_sensitive(find_button(fixture->controls, "Undo")));

    g_object_ref(row);
    gtk_editable_set_text(GTK_EDITABLE(fixture->rack), "ZZ");
    g_assert_null(gtk_widget_get_first_child(GTK_WIDGET(fixture->results)));
    g_signal_emit_by_name(fixture->results, "row-activated", row);
    g_object_unref(row);
    /* Even a stale suggestion injected into a rebuilt list is revalidated. */
    g_strlcpy(stale.word, "RETAINS", sizeof(stale.word));
    scrabble_result_list_set_results(fixture->results, &stale_results);
    click(square(fixture, 7, 4));
    click(find_button(GTK_WIDGET(result_row(fixture->results, "RETAINS")), "Place"));
    g_assert_nonnull(strstr(
        gtk_label_get_text(GTK_LABEL(fixture->status)), "current rack"));
    g_assert_false(gtk_widget_has_css_class(
        square(fixture, 7, 7), "board-tile-filled"));

    search(fixture, "RETAINS");
    click(find_button(fixture->window, "Use bundled"));
    g_assert_null(gtk_widget_get_first_child(GTK_WIDGET(fixture->results)));
}

static void records_an_opponent_move_without_using_the_user_rack(
    WindowFixture *fixture,
    gconstpointer data) {
    GtkWidget *word_entry = find_class(
        fixture->controls, "move-word-entry");
    GtkWidget *opponent_button = find_button(
        fixture->controls, "Record opponent move");
    GtkWidget *blank_entry = find_class(
        fixture->controls, "opponent-blank-entry");
    (void)data;

    gtk_editable_set_text(GTK_EDITABLE(word_entry), "RETAINS");
    click(square(fixture, 7, 4));
    g_assert_true(gtk_widget_get_sensitive(opponent_button));
    gtk_editable_set_text(GTK_EDITABLE(fixture->rack), "EEOOYNR");
    gtk_editable_set_text(GTK_EDITABLE(blank_entry), "H9");
    click(opponent_button);
    g_assert_nonnull(strstr(
        gtk_label_get_text(GTK_LABEL(fixture->status)),
        "part of the opponent's word"));
    assert_board_empty(fixture);

    gtk_editable_set_text(GTK_EDITABLE(blank_entry), "H8");
    click(opponent_button);

    assert_word(fixture, "RETAINS", 7, 4, FALSE);
    g_assert_cmpstr(
        gtk_editable_get_text(GTK_EDITABLE(fixture->rack)), ==, "EEOOYNR");
    g_assert_nonnull(strstr(
        gtk_label_get_text(GTK_LABEL(fixture->status)),
        "Opponent played RETAINS for 62 points"));
    g_assert_true(gtk_widget_has_css_class(
        square(fixture, 7, 7), "board-blank-tile"));
    g_assert_cmpstr(gtk_label_get_text(GTK_LABEL(find_class(
        square(fixture, 7, 7), "board-cell-points"))), ==, "0");
    g_assert_cmpstr(
        gtk_editable_get_text(GTK_EDITABLE(blank_entry)), ==, "");
    g_assert_null(gtk_widget_get_first_child(GTK_WIDGET(fixture->results)));

    click(find_button(fixture->controls, "Undo"));
    assert_board_empty(fixture);
}

static void clears_during_callback(
    GtkListBox *list, const ScrabbleResult *result, gpointer data) {
    int *calls = data;
    scrabble_result_list_clear(list);
    g_assert_cmpstr(result->word, ==, "RAIN");
    ++*calls;
}

static void results_own_data_and_guard_activation(void) {
    GtkListBox *list = GTK_LIST_BOX(g_object_ref_sink(scrabble_result_list_new()));
    ScrabbleResult items[2] = {0};
    ScrabbleResultSet results = {items, 2};
    GtkListBoxRow *row;
    int calls = 0;

    g_strlcpy(items[0].word, "A", sizeof(items[0].word));
    g_strlcpy(items[1].word, "RAIN", sizeof(items[1].word));
    scrabble_result_list_set_callback(list, clears_during_callback, &calls, NULL);
    scrabble_result_list_set_results(list, &results);
    memset(items, 0, sizeof(items));
    row = result_row(list, "RAIN");
    g_signal_emit_by_name(list, "row-activated", row);
    g_assert_cmpint(calls, ==, 0);
    scrabble_result_list_set_placement_available(list, TRUE);
    g_assert_false(gtk_widget_get_sensitive(find_button(
        GTK_WIDGET(result_row(list, "A")), "Place")));
    g_signal_emit_by_name(list, "row-activated", result_row(list, "A"));
    g_assert_cmpint(calls, ==, 0);
    g_signal_emit_by_name(list, "row-activated", row);
    g_assert_cmpint(calls, ==, 1);
    g_assert_null(gtk_widget_get_first_child(GTK_WIDGET(list)));
    g_object_unref(list);
}

int main(int argc, char **argv) {
    g_test_init(&argc, &argv, G_TEST_OPTION_ISOLATE_DIRS, NULL);
    if (!gtk_init_check()) {
        g_print("A display is required for GTK integration tests. Use xvfb-run.\n");
        return 77;
    }
    g_test_add("/suggestions/opening-and-connected-placement", WindowFixture, NULL,
        setup_window, places_opening_and_connected_suggestions, teardown_window);
    g_test_add("/suggestions/validation-and-invalidation", WindowFixture, NULL,
        setup_window, rejects_invalid_positions_and_revalidates_rack, teardown_window);
    g_test_add("/suggestions/opponent-recording", WindowFixture, NULL,
        setup_window, records_an_opponent_move_without_using_the_user_rack,
        teardown_window);
    g_test_add_func("/suggestions/result-lifetime", results_own_data_and_guard_activation);
    return g_test_run();
}
