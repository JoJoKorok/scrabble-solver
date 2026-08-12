#include "ui/application.h"

#include "platform/resources.h"
#include "ui/main_window.h"

#include <gtk/gtk.h>

typedef struct {
    char *dictionary_path;
    char *dictionary_error;
} ScrabbleApplicationResources;

static void destroy_resources(gpointer data, GClosure *closure) {
    ScrabbleApplicationResources *resources = data;

    (void)closure;
    g_free(resources->dictionary_path);
    g_free(resources->dictionary_error);
    g_free(resources);
}

static void activate(GtkApplication *application, gpointer user_data) {
    ScrabbleApplicationResources *resources = user_data;

    scrabble_main_window_present(
        application,
        resources->dictionary_path,
        resources->dictionary_error);
}

int scrabble_application_run(int argc, char **argv) {
    GtkApplication *application = gtk_application_new(
        "com.jojokorok.scrabblesolver",
        G_APPLICATION_DEFAULT_FLAGS);
    ScrabbleApplicationResources *resources = g_new0(
        ScrabbleApplicationResources, 1);
    GError *resource_error = NULL;
    int status;

    resources->dictionary_path = scrabble_resource_find(
        argc > 0 ? argv[0] : NULL,
        "dictionaries/demo.txt",
        &resource_error);
    if (resource_error != NULL) {
        resources->dictionary_error = g_strdup(resource_error->message);
        g_error_free(resource_error);
    }

    g_signal_connect_data(
        application,
        "activate",
        G_CALLBACK(activate),
        resources,
        destroy_resources,
        0);
    status = g_application_run(G_APPLICATION(application), argc, argv);
    g_object_unref(application);
    return status;
}
