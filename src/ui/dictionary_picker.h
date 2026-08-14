#ifndef SCRABBLE_UI_DICTIONARY_PICKER_H
#define SCRABBLE_UI_DICTIONARY_PICKER_H

#include <gtk/gtk.h>

/* The selected path and error are borrowed for the duration of the callback.
   Cancellation does not invoke the callback. */
typedef void (*ScrabbleDictionaryPickerCallback)(
    const char *path,
    const GError *error,
    gpointer user_data);

void scrabble_dictionary_picker_open(
    GtkWindow *parent,
    ScrabbleDictionaryPickerCallback callback,
    gpointer user_data,
    GDestroyNotify destroy_user_data);

#endif
