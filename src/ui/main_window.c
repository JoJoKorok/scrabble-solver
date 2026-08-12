#include "ui/main_window.h"

#include "scrabble/dictionary.h"
#include "scrabble/rack.h"
#include "scrabble/solver.h"
#include "ui/result_list.h"

enum {
    WINDOW_DEFAULT_WIDTH = 640,
    WINDOW_DEFAULT_HEIGHT = 520,
    CONTENT_SPACING = 12,
    CONTENT_MARGIN = 24
};

typedef struct {
    GtkWidget *window;
    GtkWidget *rack_entry;
    GtkWidget *solve_button;
    GtkWidget *status_label;
    GtkWidget *result_list;
    ScrabbleDictionary *dictionary;
} ScrabbleMainWindow;

static void set_status(ScrabbleMainWindow *main_window, const char *message) {
    gtk_label_set_text(GTK_LABEL(main_window->status_label), message);
}

static int read_rack(ScrabbleMainWindow *main_window, ScrabbleRack *rack) {
    const char *tiles = gtk_editable_get_text(
        GTK_EDITABLE(main_window->rack_entry));

    return tiles[0] != '\0' &&
        scrabble_rack_init(rack, tiles) == SCRABBLE_RACK_OK;
}

static void update_input_state(ScrabbleMainWindow *main_window) {
    ScrabbleRack rack;
    int valid = main_window->dictionary != NULL &&
        read_rack(main_window, &rack);

    gtk_widget_set_sensitive(main_window->solve_button, valid);
    if (main_window->dictionary == NULL) {
        set_status(main_window, "The word dictionary could not be loaded.");
    } else if (!valid) {
        set_status(main_window, "Enter one to seven letters or blank tiles.");
    } else {
        set_status(main_window, "Ready to find words.");
    }
}

static void solve(ScrabbleMainWindow *main_window) {
    ScrabbleRack rack;
    ScrabbleResultSet results;
    ScrabbleSolverStatus status;
    char message[64];

    if (!read_rack(main_window, &rack) || main_window->dictionary == NULL) {
        update_input_state(main_window);
        return;
    }

    status = scrabble_solve(main_window->dictionary, &rack, &results);
    if (status != SCRABBLE_SOLVER_OK) {
        scrabble_result_list_clear(GTK_LIST_BOX(main_window->result_list));
        set_status(main_window, "The search could not be completed.");
        return;
    }

    scrabble_result_list_set_results(
        GTK_LIST_BOX(main_window->result_list), &results);
    if (results.count == 0) {
        set_status(main_window, "No matching words found.");
    } else {
        g_snprintf(
            message,
            sizeof(message),
            "%zu matching %s found.",
            results.count,
            results.count == 1 ? "word" : "words");
        set_status(main_window, message);
    }
    scrabble_result_set_destroy(&results);
}

static void on_rack_changed(GtkEditable *editable, gpointer user_data) {
    ScrabbleMainWindow *main_window = user_data;

    (void)editable;
    scrabble_result_list_clear(GTK_LIST_BOX(main_window->result_list));
    update_input_state(main_window);
}

static void on_rack_activated(GtkEntry *entry, gpointer user_data) {
    (void)entry;
    solve(user_data);
}

static void on_solve_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    solve(user_data);
}

static void destroy_main_window(gpointer data) {
    ScrabbleMainWindow *main_window = data;

    scrabble_dictionary_destroy(main_window->dictionary);
    g_free(main_window);
}

static GtkWidget *create_content(ScrabbleMainWindow *main_window) {
    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, CONTENT_SPACING);
    GtkWidget *title = gtk_label_new("Scrabble Solver");
    GtkWidget *instructions = gtk_label_new(
        "Enter up to seven rack tiles. Use ? or * for a blank tile.");
    GtkWidget *controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *results_scroll = gtk_scrolled_window_new();
    main_window->rack_entry = gtk_entry_new();
    main_window->solve_button = gtk_button_new_with_label("Find words");
    main_window->status_label = gtk_label_new(
        "Word suggestions will appear here in the next version.");

    gtk_widget_set_margin_top(content, CONTENT_MARGIN);
    gtk_widget_set_margin_bottom(content, CONTENT_MARGIN);
    gtk_widget_set_margin_start(content, CONTENT_MARGIN);
    gtk_widget_set_margin_end(content, CONTENT_MARGIN);

    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_widget_add_css_class(title, "title-1");
    gtk_box_append(GTK_BOX(content), title);

    gtk_widget_set_halign(instructions, GTK_ALIGN_START);
    gtk_label_set_wrap(GTK_LABEL(instructions), TRUE);
    gtk_box_append(GTK_BOX(content), instructions);

    gtk_entry_set_max_length(GTK_ENTRY(main_window->rack_entry), 7);
    gtk_entry_set_placeholder_text(
        GTK_ENTRY(main_window->rack_entry), "Example: RETAINS");
    gtk_widget_set_hexpand(main_window->rack_entry, TRUE);
    gtk_accessible_update_property(
        GTK_ACCESSIBLE(main_window->rack_entry),
        GTK_ACCESSIBLE_PROPERTY_LABEL, "Rack tiles",
        -1);
    gtk_box_append(GTK_BOX(controls), main_window->rack_entry);

    gtk_widget_set_sensitive(main_window->solve_button, FALSE);
    gtk_box_append(GTK_BOX(controls), main_window->solve_button);
    gtk_box_append(GTK_BOX(content), controls);

    gtk_widget_set_halign(main_window->status_label, GTK_ALIGN_START);
    gtk_label_set_wrap(GTK_LABEL(main_window->status_label), TRUE);
    gtk_box_append(GTK_BOX(content), main_window->status_label);

    main_window->result_list = scrabble_result_list_new();
    gtk_widget_set_vexpand(results_scroll, TRUE);
    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(results_scroll),
        GTK_POLICY_NEVER,
        GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(results_scroll), main_window->result_list);
    gtk_box_append(GTK_BOX(content), results_scroll);

    g_signal_connect(
        main_window->rack_entry,
        "changed",
        G_CALLBACK(on_rack_changed),
        main_window);
    g_signal_connect(
        main_window->rack_entry,
        "activate",
        G_CALLBACK(on_rack_activated),
        main_window);
    g_signal_connect(
        main_window->solve_button,
        "clicked",
        G_CALLBACK(on_solve_clicked),
        main_window);

    return content;
}

void scrabble_main_window_present(GtkApplication *application) {
    GtkWindow *active_window = gtk_application_get_active_window(application);
    ScrabbleMainWindow *main_window;

    if (active_window != NULL) {
        gtk_window_present(active_window);
        return;
    }

    main_window = g_new0(ScrabbleMainWindow, 1);
    main_window->dictionary = scrabble_dictionary_load(
        "assets/dictionaries/demo.txt", NULL);
    main_window->window = gtk_application_window_new(application);
    g_object_set_data_full(
        G_OBJECT(main_window->window),
        "scrabble-main-window",
        main_window,
        destroy_main_window);

    gtk_window_set_title(GTK_WINDOW(main_window->window), "Scrabble Solver");
    gtk_window_set_default_size(
        GTK_WINDOW(main_window->window),
        WINDOW_DEFAULT_WIDTH,
        WINDOW_DEFAULT_HEIGHT);
    gtk_window_set_child(
        GTK_WINDOW(main_window->window), create_content(main_window));
    update_input_state(main_window);
    gtk_window_present(GTK_WINDOW(main_window->window));
}
