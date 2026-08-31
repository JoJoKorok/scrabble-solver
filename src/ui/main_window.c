#include "ui/main_window.h"

#include "platform/settings.h"
#include "scrabble/dictionary.h"
#include "scrabble/game.h"
#include "scrabble/rack.h"
#include "scrabble/solver.h"
#include "ui/board_view.h"
#include "ui/dictionary_picker.h"
#include "ui/rack_view.h"
#include "ui/result_list.h"

enum {
    WINDOW_DEFAULT_WIDTH = 1180,
    WINDOW_DEFAULT_HEIGHT = 820,
    WINDOW_MINIMUM_WIDTH = 900,
    WINDOW_MINIMUM_HEIGHT = 620,
    CONTENT_SPACING = 18
};

typedef struct {
    GtkWidget *window;
    GtkWidget *board_view;
    GtkWidget *rack_entry;
    GtkWidget *rack_view;
    GtkWidget *solve_button;
    GtkWidget *status_label;
    GtkWidget *result_list;
    GtkWidget *dictionary_label;
    GtkWidget *reset_dictionary_button;
    ScrabbleGame *game;
    ScrabbleDictionary *dictionary;
    char *dictionary_error;
    char *bundled_dictionary_path;
    char *active_dictionary_path;
    char *settings_path;
    gboolean using_custom_dictionary;
    gboolean has_saved_dictionary_preference;
} ScrabbleMainWindow;

static void set_status(
    ScrabbleMainWindow *main_window,
    const char *message,
    const char *style_class) {
    static const char *classes[] = {
        "status-muted", "status-ready", "status-success", "status-error"
    };

    for (size_t index = 0; index < G_N_ELEMENTS(classes); ++index) {
        gtk_widget_remove_css_class(main_window->status_label, classes[index]);
    }
    gtk_label_set_text(GTK_LABEL(main_window->status_label), message);
    gtk_widget_add_css_class(main_window->status_label, style_class);
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
        set_status(
            main_window,
            main_window->dictionary_error == NULL
                ? "No dictionary is available. Choose a word-list file."
                : main_window->dictionary_error,
            "status-error");
    } else if (!valid) {
        set_status(
            main_window,
            "Enter one to seven letters or blank tiles.",
            "status-muted");
    } else {
        set_status(main_window, "Ready to find words.", "status-ready");
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
        set_status(
            main_window,
            "The search could not be completed.",
            "status-error");
        return;
    }

    scrabble_result_list_set_results(
        GTK_LIST_BOX(main_window->result_list), &results);
    if (results.count == 0) {
        set_status(main_window, "No matching words found.", "status-muted");
    } else {
        g_snprintf(
            message,
            sizeof(message),
            "%zu matching %s found.",
            results.count,
            results.count == 1 ? "word" : "words");
        set_status(main_window, message, "status-success");
    }
    scrabble_result_set_destroy(&results);
}

static void on_rack_changed(GtkEditable *editable, gpointer user_data) {
    ScrabbleMainWindow *main_window = user_data;

    (void)editable;
    scrabble_rack_view_set_tiles(
        main_window->rack_view,
        gtk_editable_get_text(GTK_EDITABLE(main_window->rack_entry)));
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

    scrabble_game_destroy(main_window->game);
    scrabble_dictionary_destroy(main_window->dictionary);
    g_free(main_window->dictionary_error);
    g_free(main_window->bundled_dictionary_path);
    g_free(main_window->active_dictionary_path);
    g_free(main_window->settings_path);
    g_free(main_window);
}

static const char *dictionary_load_error(ScrabbleDictionaryStatus status) {
    switch (status) {
        case SCRABBLE_DICTIONARY_OPEN_FAILED:
            return "The dictionary was found but could not be opened.";
        case SCRABBLE_DICTIONARY_READ_FAILED:
            return "The dictionary could not be read completely.";
        case SCRABBLE_DICTIONARY_OUT_OF_MEMORY:
            return "There is not enough memory to load the dictionary.";
        default:
            return "The dictionary could not be loaded.";
    }
}

static void update_dictionary_summary(
    ScrabbleMainWindow *main_window,
    const char *note) {
    char *display_name;
    char *message;

    gtk_widget_remove_css_class(
        main_window->dictionary_label, "dictionary-warning");
    if (main_window->dictionary == NULL ||
        main_window->active_dictionary_path == NULL) {
        gtk_label_set_text(
            GTK_LABEL(main_window->dictionary_label),
            note == NULL ? "No dictionary loaded." : note);
        gtk_widget_set_tooltip_text(main_window->dictionary_label, NULL);
        gtk_widget_set_sensitive(main_window->reset_dictionary_button, FALSE);
        if (note != NULL) {
            gtk_widget_add_css_class(
                main_window->dictionary_label, "dictionary-warning");
        }
        return;
    }

    display_name = g_filename_display_basename(
        main_window->active_dictionary_path);
    if (note == NULL) {
        message = g_strdup_printf(
            "%s: %s — %zu words",
            main_window->using_custom_dictionary ? "Custom" : "Bundled",
            display_name,
            scrabble_dictionary_count(main_window->dictionary));
    } else {
        message = g_strdup_printf(
            "%s: %s — %zu words. %s",
            main_window->using_custom_dictionary ? "Custom" : "Bundled",
            display_name,
            scrabble_dictionary_count(main_window->dictionary),
            note);
        gtk_widget_add_css_class(
            main_window->dictionary_label, "dictionary-warning");
    }

    gtk_label_set_text(GTK_LABEL(main_window->dictionary_label), message);
    gtk_widget_set_tooltip_text(
        main_window->dictionary_label, main_window->active_dictionary_path);
    gtk_widget_set_sensitive(
        main_window->reset_dictionary_button,
        main_window->using_custom_dictionary ||
            main_window->has_saved_dictionary_preference);
    g_free(message);
    g_free(display_name);
}

static ScrabbleDictionary *load_dictionary_candidate(
    const char *path,
    char **error_message) {
    ScrabbleDictionaryStatus status;
    ScrabbleDictionary *dictionary;

    g_return_val_if_fail(error_message != NULL, NULL);
    *error_message = NULL;
    if (path == NULL || path[0] == '\0') {
        *error_message = g_strdup("No dictionary file was selected.");
        return NULL;
    }

    dictionary = scrabble_dictionary_load(path, &status);
    if (dictionary == NULL) {
        *error_message = g_strdup(dictionary_load_error(status));
        return NULL;
    }
    if (scrabble_dictionary_count(dictionary) == 0) {
        scrabble_dictionary_destroy(dictionary);
        *error_message = g_strdup(
            "The selected file contains no supported words.");
        return NULL;
    }

    return dictionary;
}

static gboolean activate_dictionary(
    ScrabbleMainWindow *main_window,
    const char *path,
    gboolean custom,
    const char *note,
    char **error_message) {
    ScrabbleDictionary *dictionary = load_dictionary_candidate(
        path, error_message);

    if (dictionary == NULL) {
        return FALSE;
    }

    scrabble_dictionary_destroy(main_window->dictionary);
    main_window->dictionary = dictionary;
    g_free(main_window->active_dictionary_path);
    main_window->active_dictionary_path = g_strdup(path);
    main_window->using_custom_dictionary = custom;
    g_clear_pointer(&main_window->dictionary_error, g_free);
    scrabble_result_list_clear(GTK_LIST_BOX(main_window->result_list));
    update_dictionary_summary(main_window, note);
    update_input_state(main_window);
    return TRUE;
}

static void load_initial_dictionary(
    ScrabbleMainWindow *main_window,
    const char *resource_error) {
    GError *settings_error = NULL;
    char *saved_path = scrabble_settings_load_dictionary_path(
        main_window->settings_path, &settings_error);
    char *load_error = NULL;
    char *fallback_note = NULL;

    if (settings_error != NULL) {
        fallback_note = g_strdup(
            "Saved settings could not be read; using the bundled dictionary.");
        g_clear_error(&settings_error);
    } else if (saved_path != NULL) {
        main_window->has_saved_dictionary_preference = TRUE;
        if (activate_dictionary(
                main_window, saved_path, TRUE, NULL, &load_error)) {
            g_free(saved_path);
            return;
        }

        fallback_note = g_strdup_printf(
            "Saved dictionary unavailable (%s); using the bundled dictionary.",
            load_error);
        g_clear_pointer(&load_error, g_free);
    }

    if (main_window->bundled_dictionary_path != NULL &&
        activate_dictionary(
            main_window,
            main_window->bundled_dictionary_path,
            FALSE,
            fallback_note,
            &load_error)) {
        g_free(fallback_note);
        g_free(saved_path);
        return;
    }

    main_window->dictionary_error = g_strdup(
        load_error != NULL
            ? load_error
            : resource_error == NULL
                ? "The bundled dictionary resource is unavailable."
                : resource_error);
    update_dictionary_summary(main_window, fallback_note);
    update_input_state(main_window);
    g_free(load_error);
    g_free(fallback_note);
    g_free(saved_path);
}

static void select_custom_dictionary(
    ScrabbleMainWindow *main_window,
    const char *path) {
    GError *settings_error = NULL;
    char *load_error = NULL;

    if (!activate_dictionary(
            main_window, path, TRUE, NULL, &load_error)) {
        set_status(main_window, load_error, "status-error");
        g_free(load_error);
        return;
    }

    if (!scrabble_settings_save_dictionary_path(
            main_window->settings_path, path, &settings_error)) {
        update_dictionary_summary(
            main_window,
            "Loaded for this session, but the choice could not be remembered.");
        set_status(
            main_window,
            settings_error == NULL
                ? "The dictionary choice could not be remembered."
                : settings_error->message,
            "status-error");
        g_clear_error(&settings_error);
        return;
    }

    main_window->has_saved_dictionary_preference = TRUE;
    update_dictionary_summary(main_window, NULL);
    set_status(main_window, "Custom dictionary loaded.", "status-success");
}

static void on_dictionary_selected(
    const char *path,
    const GError *error,
    gpointer user_data) {
    GtkWindow *window = GTK_WINDOW(user_data);
    ScrabbleMainWindow *main_window = g_object_get_data(
        G_OBJECT(window), "scrabble-main-window");

    if (main_window == NULL) {
        return;
    }
    if (error != NULL) {
        set_status(main_window, error->message, "status-error");
        return;
    }

    select_custom_dictionary(main_window, path);
}

static void on_choose_dictionary_clicked(
    GtkButton *button,
    gpointer user_data) {
    ScrabbleMainWindow *main_window = user_data;

    (void)button;
    scrabble_dictionary_picker_open(
        GTK_WINDOW(main_window->window),
        on_dictionary_selected,
        g_object_ref(main_window->window),
        g_object_unref);
}

static void on_reset_dictionary_clicked(
    GtkButton *button,
    gpointer user_data) {
    ScrabbleMainWindow *main_window = user_data;
    GError *settings_error = NULL;
    char *load_error = NULL;

    (void)button;
    if (main_window->bundled_dictionary_path == NULL ||
        !activate_dictionary(
            main_window,
            main_window->bundled_dictionary_path,
            FALSE,
            NULL,
            &load_error)) {
        set_status(
            main_window,
            load_error == NULL
                ? "The bundled dictionary is unavailable."
                : load_error,
            "status-error");
        g_free(load_error);
        return;
    }

    if (!scrabble_settings_clear_dictionary_path(
            main_window->settings_path, &settings_error)) {
        update_dictionary_summary(
            main_window,
            "Restored, but the saved preference could not be cleared.");
        set_status(
            main_window,
            settings_error == NULL
                ? "The saved dictionary preference could not be cleared."
                : settings_error->message,
            "status-error");
        g_clear_error(&settings_error);
        return;
    }

    main_window->has_saved_dictionary_preference = FALSE;
    update_dictionary_summary(main_window, NULL);
    set_status(main_window, "Bundled dictionary restored.", "status-success");
}

static GtkWidget *create_content(ScrabbleMainWindow *main_window) {
    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, CONTENT_SPACING);
    GtkWidget *title = gtk_label_new("Scrabble Solver");
    GtkWidget *instructions = gtk_label_new(
        "Build the board as you play, then turn your rack into the strongest move.");
    GtkWidget *dictionary_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *dictionary_heading = gtk_label_new("DICTIONARY");
    GtkWidget *dictionary_controls = gtk_box_new(
        GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *choose_dictionary_button = gtk_button_new_with_label(
        "Choose file");
    GtkWidget *workspace = gtk_box_new(
        GTK_ORIENTATION_HORIZONTAL, CONTENT_SPACING);
    GtkWidget *board_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *board_heading = gtk_label_new("GAME BOARD");
    GtkWidget *board_note = gtk_label_new(
        "Select a square to prepare the starting position. Move entry is the next step.");
    GtkWidget *board_scroll = gtk_scrolled_window_new();
    GtkWidget *sidebar = gtk_box_new(
        GTK_ORIENTATION_VERTICAL, CONTENT_SPACING);
    GtkWidget *rack_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *rack_heading = gtk_label_new("YOUR RACK");
    GtkWidget *controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *results_heading = gtk_label_new("WORD SUGGESTIONS");
    GtkWidget *results_scroll = gtk_scrolled_window_new();
    main_window->rack_entry = gtk_entry_new();
    main_window->solve_button = gtk_button_new_with_label("Find words");
    main_window->status_label = gtk_label_new(
        "Word suggestions will appear here in the next version.");
    main_window->dictionary_label = gtk_label_new("Loading dictionary…");
    main_window->reset_dictionary_button = gtk_button_new_with_label(
        "Use bundled");

    gtk_widget_add_css_class(content, "app-background");
    gtk_widget_add_css_class(content, "app-content");

    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_widget_add_css_class(title, "app-title");
    gtk_box_append(GTK_BOX(content), title);

    gtk_widget_set_halign(instructions, GTK_ALIGN_START);
    gtk_label_set_wrap(GTK_LABEL(instructions), TRUE);
    gtk_widget_add_css_class(instructions, "app-subtitle");
    gtk_box_append(GTK_BOX(content), instructions);

    gtk_widget_add_css_class(dictionary_card, "surface-card");
    gtk_widget_add_css_class(dictionary_card, "dictionary-card");
    gtk_widget_set_halign(dictionary_heading, GTK_ALIGN_START);
    gtk_widget_add_css_class(dictionary_heading, "section-label");
    gtk_box_append(GTK_BOX(dictionary_card), dictionary_heading);

    gtk_widget_set_halign(main_window->dictionary_label, GTK_ALIGN_START);
    gtk_widget_set_hexpand(main_window->dictionary_label, TRUE);
    gtk_label_set_wrap(GTK_LABEL(main_window->dictionary_label), TRUE);
    gtk_widget_add_css_class(
        main_window->dictionary_label, "dictionary-label");
    gtk_box_append(
        GTK_BOX(dictionary_controls), main_window->dictionary_label);

    gtk_widget_add_css_class(choose_dictionary_button, "secondary-button");
    gtk_accessible_update_property(
        GTK_ACCESSIBLE(choose_dictionary_button),
        GTK_ACCESSIBLE_PROPERTY_LABEL, "Choose a dictionary file",
        -1);
    gtk_box_append(GTK_BOX(dictionary_controls), choose_dictionary_button);

    gtk_widget_set_sensitive(main_window->reset_dictionary_button, FALSE);
    gtk_widget_add_css_class(
        main_window->reset_dictionary_button, "secondary-button");
    gtk_box_append(
        GTK_BOX(dictionary_controls), main_window->reset_dictionary_button);
    gtk_box_append(GTK_BOX(dictionary_card), dictionary_controls);
    gtk_box_append(GTK_BOX(content), dictionary_card);

    gtk_widget_set_hexpand(workspace, TRUE);
    gtk_widget_set_vexpand(workspace, TRUE);

    gtk_widget_add_css_class(board_card, "surface-card");
    gtk_widget_add_css_class(board_card, "board-card");
    gtk_widget_set_hexpand(board_card, TRUE);
    gtk_widget_set_vexpand(board_card, TRUE);
    gtk_widget_set_halign(board_heading, GTK_ALIGN_START);
    gtk_widget_add_css_class(board_heading, "section-label");
    gtk_box_append(GTK_BOX(board_card), board_heading);

    gtk_widget_set_halign(board_note, GTK_ALIGN_START);
    gtk_label_set_wrap(GTK_LABEL(board_note), TRUE);
    gtk_widget_add_css_class(board_note, "board-note");
    gtk_box_append(GTK_BOX(board_card), board_note);

    main_window->board_view = scrabble_board_view_new();
    scrabble_board_view_set_board(
        main_window->board_view,
        main_window->game == NULL
            ? NULL
            : scrabble_game_board(main_window->game));
    gtk_widget_set_vexpand(board_scroll, TRUE);
    gtk_widget_add_css_class(board_scroll, "board-scroll");
    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(board_scroll),
        GTK_POLICY_AUTOMATIC,
        GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_min_content_width(
        GTK_SCROLLED_WINDOW(board_scroll), 480);
    gtk_scrolled_window_set_min_content_height(
        GTK_SCROLLED_WINDOW(board_scroll), 440);
    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(board_scroll), main_window->board_view);
    gtk_box_append(GTK_BOX(board_card), board_scroll);
    gtk_box_append(GTK_BOX(workspace), board_card);

    gtk_widget_set_size_request(sidebar, 350, -1);
    gtk_widget_set_vexpand(sidebar, TRUE);
    gtk_box_append(GTK_BOX(workspace), sidebar);
    gtk_box_append(GTK_BOX(content), workspace);

    gtk_widget_add_css_class(rack_card, "surface-card");
    gtk_widget_set_halign(rack_heading, GTK_ALIGN_START);
    gtk_widget_add_css_class(rack_heading, "section-label");
    gtk_box_append(GTK_BOX(rack_card), rack_heading);

    main_window->rack_view = scrabble_rack_view_new();
    gtk_box_append(GTK_BOX(rack_card), main_window->rack_view);

    gtk_entry_set_max_length(GTK_ENTRY(main_window->rack_entry), 7);
    gtk_entry_set_placeholder_text(
        GTK_ENTRY(main_window->rack_entry), "Example: RETAINS");
    gtk_widget_set_hexpand(main_window->rack_entry, TRUE);
    gtk_widget_add_css_class(main_window->rack_entry, "rack-entry");
    gtk_accessible_update_property(
        GTK_ACCESSIBLE(main_window->rack_entry),
        GTK_ACCESSIBLE_PROPERTY_LABEL, "Rack tiles",
        -1);
    gtk_box_append(GTK_BOX(controls), main_window->rack_entry);

    gtk_widget_set_sensitive(main_window->solve_button, FALSE);
    gtk_widget_add_css_class(main_window->solve_button, "solve-button");
    gtk_box_append(GTK_BOX(controls), main_window->solve_button);
    gtk_box_append(GTK_BOX(rack_card), controls);

    gtk_widget_set_halign(main_window->status_label, GTK_ALIGN_START);
    gtk_label_set_wrap(GTK_LABEL(main_window->status_label), TRUE);
    gtk_widget_add_css_class(main_window->status_label, "status-label");
    gtk_box_append(GTK_BOX(rack_card), main_window->status_label);
    gtk_box_append(GTK_BOX(sidebar), rack_card);

    gtk_widget_set_halign(results_heading, GTK_ALIGN_START);
    gtk_widget_add_css_class(results_heading, "section-label");
    gtk_box_append(GTK_BOX(sidebar), results_heading);

    main_window->result_list = scrabble_result_list_new();
    gtk_widget_set_vexpand(results_scroll, TRUE);
    gtk_widget_add_css_class(results_scroll, "results-scroll");
    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(results_scroll),
        GTK_POLICY_NEVER,
        GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(results_scroll), main_window->result_list);
    gtk_box_append(GTK_BOX(sidebar), results_scroll);

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
    g_signal_connect(
        choose_dictionary_button,
        "clicked",
        G_CALLBACK(on_choose_dictionary_clicked),
        main_window);
    g_signal_connect(
        main_window->reset_dictionary_button,
        "clicked",
        G_CALLBACK(on_reset_dictionary_clicked),
        main_window);

    return content;
}

void scrabble_main_window_present(
    GtkApplication *application,
    const char *dictionary_path,
    const char *resource_error) {
    GtkWindow *active_window = gtk_application_get_active_window(application);
    ScrabbleMainWindow *main_window;

    if (active_window != NULL) {
        gtk_window_present(active_window);
        return;
    }

    main_window = g_new0(ScrabbleMainWindow, 1);
    main_window->game = scrabble_game_create();
    main_window->bundled_dictionary_path = g_strdup(dictionary_path);
    main_window->settings_path = scrabble_settings_default_path();
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
    gtk_widget_set_size_request(
        main_window->window,
        WINDOW_MINIMUM_WIDTH,
        WINDOW_MINIMUM_HEIGHT);
    gtk_window_set_child(
        GTK_WINDOW(main_window->window), create_content(main_window));
    load_initial_dictionary(main_window, resource_error);
    gtk_window_present(GTK_WINDOW(main_window->window));
}
