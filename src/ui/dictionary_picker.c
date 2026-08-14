#include "ui/dictionary_picker.h"

typedef struct {
    ScrabbleDictionaryPickerCallback callback;
    gpointer user_data;
    GDestroyNotify destroy_user_data;
} ScrabbleDictionaryPickerRequest;

static ScrabbleDictionaryPickerRequest *request_new(
    ScrabbleDictionaryPickerCallback callback,
    gpointer user_data,
    GDestroyNotify destroy_user_data) {
    ScrabbleDictionaryPickerRequest *request = g_new0(
        ScrabbleDictionaryPickerRequest, 1);

    request->callback = callback;
    request->user_data = user_data;
    request->destroy_user_data = destroy_user_data;
    return request;
}

static void request_free(ScrabbleDictionaryPickerRequest *request) {
    if (request->destroy_user_data != NULL) {
        request->destroy_user_data(request->user_data);
    }
    g_free(request);
}

static void deliver_selected_file(
    ScrabbleDictionaryPickerRequest *request,
    GFile *file) {
    GError *error = NULL;
    char *path = g_file_get_path(file);

    if (path == NULL) {
        g_set_error_literal(
            &error,
            G_IO_ERROR,
            G_IO_ERROR_NOT_SUPPORTED,
            "Choose a dictionary stored on this computer.");
    }

    request->callback(path, error, request->user_data);
    g_clear_error(&error);
    g_free(path);
}

static GtkFileFilter *create_word_list_filter(void) {
    GtkFileFilter *filter = gtk_file_filter_new();

    if (g_object_is_floating(filter)) {
        g_object_ref_sink(filter);
    }
    gtk_file_filter_set_name(filter, "Word lists");
    gtk_file_filter_add_pattern(filter, "*.txt");
    gtk_file_filter_add_pattern(filter, "*.dic");
    gtk_file_filter_add_pattern(filter, "*.lst");
    return filter;
}

static GtkFileFilter *create_all_files_filter(void) {
    GtkFileFilter *filter = gtk_file_filter_new();

    if (g_object_is_floating(filter)) {
        g_object_ref_sink(filter);
    }
    gtk_file_filter_set_name(filter, "All files");
    gtk_file_filter_add_pattern(filter, "*");
    return filter;
}

#if GTK_CHECK_VERSION(4, 10, 0)
static void on_dictionary_dialog_opened(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data) {
    ScrabbleDictionaryPickerRequest *request = user_data;
    GError *error = NULL;
    GFile *file = gtk_file_dialog_open_finish(
        GTK_FILE_DIALOG(source_object), result, &error);

    if (file != NULL) {
        deliver_selected_file(request, file);
    } else if (!g_error_matches(
                   error, GTK_DIALOG_ERROR, GTK_DIALOG_ERROR_DISMISSED)) {
        request->callback(NULL, error, request->user_data);
    }

    g_clear_object(&file);
    g_clear_error(&error);
    request_free(request);
}

void scrabble_dictionary_picker_open(
    GtkWindow *parent,
    ScrabbleDictionaryPickerCallback callback,
    gpointer user_data,
    GDestroyNotify destroy_user_data) {
    GtkFileDialog *dialog;
    GtkFileFilter *word_lists;
    GtkFileFilter *all_files;
    GListStore *filters;
    ScrabbleDictionaryPickerRequest *request;

    g_return_if_fail(GTK_IS_WINDOW(parent));
    g_return_if_fail(callback != NULL);

    dialog = gtk_file_dialog_new();
    word_lists = create_word_list_filter();
    all_files = create_all_files_filter();
    filters = g_list_store_new(GTK_TYPE_FILE_FILTER);
    request = request_new(callback, user_data, destroy_user_data);

    g_list_store_append(filters, word_lists);
    g_list_store_append(filters, all_files);
    gtk_file_dialog_set_title(dialog, "Choose a dictionary");
    gtk_file_dialog_set_modal(dialog, TRUE);
    gtk_file_dialog_set_filters(dialog, G_LIST_MODEL(filters));
    gtk_file_dialog_set_default_filter(dialog, word_lists);
    gtk_file_dialog_open(
        dialog, parent, NULL, on_dictionary_dialog_opened, request);

    g_object_unref(filters);
    g_object_unref(all_files);
    g_object_unref(word_lists);
    g_object_unref(dialog);
}
#else
static void on_native_dictionary_response(
    GtkNativeDialog *dialog,
    int response,
    gpointer user_data) {
    ScrabbleDictionaryPickerRequest *request = user_data;

    if (response == GTK_RESPONSE_ACCEPT) {
        GFile *file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));

        if (file != NULL) {
            deliver_selected_file(request, file);
            g_object_unref(file);
        }
    }

    request_free(request);
    g_object_unref(dialog);
}

void scrabble_dictionary_picker_open(
    GtkWindow *parent,
    ScrabbleDictionaryPickerCallback callback,
    gpointer user_data,
    GDestroyNotify destroy_user_data) {
    GtkFileChooserNative *dialog;
    GtkFileFilter *word_lists;
    GtkFileFilter *all_files;
    ScrabbleDictionaryPickerRequest *request;

    g_return_if_fail(GTK_IS_WINDOW(parent));
    g_return_if_fail(callback != NULL);

    dialog = gtk_file_chooser_native_new(
        "Choose a dictionary",
        parent,
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "Open",
        "Cancel");
    word_lists = create_word_list_filter();
    all_files = create_all_files_filter();
    request = request_new(callback, user_data, destroy_user_data);

    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), word_lists);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), all_files);
    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), word_lists);
    g_signal_connect(
        dialog,
        "response",
        G_CALLBACK(on_native_dictionary_response),
        request);
    gtk_native_dialog_show(GTK_NATIVE_DIALOG(dialog));

    g_object_unref(all_files);
    g_object_unref(word_lists);
}
#endif
