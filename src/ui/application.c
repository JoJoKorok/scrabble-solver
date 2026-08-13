#include "ui/application.h"

#include "platform/resources.h"
#include "ui/main_window.h"
#include "ui/theme.h"

#include <gtk/gtk.h>

typedef struct {
    char *dictionary_path;
    char *dictionary_error;
    char *stylesheet_path;
} ScrabbleApplicationResources;

static void destroy_resources(gpointer data, GClosure *closure) {
    ScrabbleApplicationResources *resources = data;

    (void)closure;
    g_free(resources->dictionary_path);
    g_free(resources->dictionary_error);
    g_free(resources->stylesheet_path);
    g_free(resources);
}

static void activate(GtkApplication *application, gpointer user_data) {
    ScrabbleApplicationResources *resources = user_data;
    GError *theme_error = NULL;

    if (resources->stylesheet_path != NULL &&
        !scrabble_theme_apply(resources->stylesheet_path, &theme_error)) {
        g_warning("Could not apply application theme: %s",
                  theme_error == NULL ? "unknown error" : theme_error->message);
        g_clear_error(&theme_error);
    }

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
        SCRABBLE_DEFAULT_DICTIONARY_PATH,
        &resource_error);
    if (resource_error != NULL) {
        resources->dictionary_error = g_strdup(resource_error->message);
        g_error_free(resource_error);
    }
    resources->stylesheet_path = scrabble_resource_find(
        argc > 0 ? argv[0] : NULL,
        "styles/application.css",
        NULL);
    if (resources->stylesheet_path == NULL) {
        resources->stylesheet_path = scrabble_resource_find_bundled(
            argc > 0 ? argv[0] : NULL,
            "styles/application.css",
            NULL);
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
