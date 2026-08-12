#include "ui/application.h"

#include "ui/main_window.h"

#include <gtk/gtk.h>

static void activate(GtkApplication *application, gpointer user_data) {
    (void)user_data;
    scrabble_main_window_present(application);
}

int scrabble_application_run(int argc, char **argv) {
    GtkApplication *application = gtk_application_new(
        "com.jojokorok.scrabblesolver",
        G_APPLICATION_DEFAULT_FLAGS);
    int status;

    g_signal_connect(application, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(application), argc, argv);
    g_object_unref(application);
    return status;
}
