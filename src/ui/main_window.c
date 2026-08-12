#include "ui/main_window.h"

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
} ScrabbleMainWindow;

static GtkWidget *create_content(ScrabbleMainWindow *main_window) {
    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, CONTENT_SPACING);
    GtkWidget *title = gtk_label_new("Scrabble Solver");
    GtkWidget *instructions = gtk_label_new(
        "Enter up to seven rack tiles. Use ? or * for a blank tile.");
    GtkWidget *controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
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
    gtk_widget_set_tooltip_text(
        main_window->solve_button,
        "Word search will be connected in the next development step.");
    gtk_box_append(GTK_BOX(controls), main_window->solve_button);
    gtk_box_append(GTK_BOX(content), controls);

    gtk_widget_set_halign(main_window->status_label, GTK_ALIGN_START);
    gtk_label_set_wrap(GTK_LABEL(main_window->status_label), TRUE);
    gtk_box_append(GTK_BOX(content), main_window->status_label);

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
    main_window->window = gtk_application_window_new(application);
    g_object_set_data_full(
        G_OBJECT(main_window->window),
        "scrabble-main-window",
        main_window,
        g_free);

    gtk_window_set_title(GTK_WINDOW(main_window->window), "Scrabble Solver");
    gtk_window_set_default_size(
        GTK_WINDOW(main_window->window),
        WINDOW_DEFAULT_WIDTH,
        WINDOW_DEFAULT_HEIGHT);
    gtk_window_set_child(
        GTK_WINDOW(main_window->window), create_content(main_window));
    gtk_window_present(GTK_WINDOW(main_window->window));
}
