#include "ui/theme.h"

int scrabble_theme_apply(const char *stylesheet_path, GError **error) {
    GtkCssProvider *provider;
    GdkDisplay *display;
    char *stylesheet;
    gsize stylesheet_length;

    g_return_val_if_fail(error == NULL || *error == NULL, 0);
    if (stylesheet_path == NULL || stylesheet_path[0] == '\0') {
        return 0;
    }

    if (!g_file_get_contents(
            stylesheet_path,
            &stylesheet,
            &stylesheet_length,
            error)) {
        return 0;
    }
    (void)stylesheet_length;

    display = gdk_display_get_default();
    if (display == NULL) {
        g_free(stylesheet);
        return 0;
    }

    provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider, stylesheet);
    gtk_style_context_add_provider_for_display(
        display,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
    g_free(stylesheet);
    return 1;
}
