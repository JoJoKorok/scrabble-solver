#include "ui/board_view.h"

#include "scrabble/scoring.h"

enum {
    BOARD_CELL_SIZE = 31,
    BOARD_CELL_COUNT = SCRABBLE_BOARD_SIZE * SCRABBLE_BOARD_SIZE
};

typedef struct ScrabbleBoardViewState ScrabbleBoardViewState;

typedef struct {
    ScrabbleBoardViewState *view;
    ScrabbleBoardPosition position;
    GtkWidget *button;
    GtkWidget *primary_label;
    GtkWidget *secondary_label;
} ScrabbleBoardViewCell;

struct ScrabbleBoardViewState {
    GtkWidget *widget;
    ScrabbleBoardViewCell cells[BOARD_CELL_COUNT];
    ScrabbleBoardPosition selected_position;
    gboolean has_selection;
    ScrabbleBoardViewSelectionCallback selection_callback;
    gpointer selection_data;
    GDestroyNotify selection_data_destroy;
};

static size_t cell_index(ScrabbleBoardPosition position) {
    return position.row * SCRABBLE_BOARD_SIZE + position.column;
}

static ScrabbleBoardViewState *board_view_state(GtkWidget *board_view) {
    if (!GTK_IS_GRID(board_view)) {
        return NULL;
    }

    return g_object_get_data(G_OBJECT(board_view), "scrabble-board-view");
}

static const char *premium_class(ScrabbleBoardPremium premium) {
    switch (premium) {
        case SCRABBLE_BOARD_DOUBLE_LETTER:
            return "board-double-letter";
        case SCRABBLE_BOARD_TRIPLE_LETTER:
            return "board-triple-letter";
        case SCRABBLE_BOARD_DOUBLE_WORD:
            return "board-double-word";
        case SCRABBLE_BOARD_TRIPLE_WORD:
            return "board-triple-word";
        default:
            return "board-normal-square";
    }
}

static const char *premium_text(
    ScrabbleBoardPosition position,
    ScrabbleBoardPremium premium) {
    if (scrabble_board_position_is_center(position)) {
        return "★";
    }

    switch (premium) {
        case SCRABBLE_BOARD_DOUBLE_LETTER:
            return "2L";
        case SCRABBLE_BOARD_TRIPLE_LETTER:
            return "3L";
        case SCRABBLE_BOARD_DOUBLE_WORD:
            return "2W";
        case SCRABBLE_BOARD_TRIPLE_WORD:
            return "3W";
        default:
            return "";
    }
}

static const char *premium_description(ScrabbleBoardPremium premium) {
    switch (premium) {
        case SCRABBLE_BOARD_DOUBLE_LETTER:
            return "double letter score";
        case SCRABBLE_BOARD_TRIPLE_LETTER:
            return "triple letter score";
        case SCRABBLE_BOARD_DOUBLE_WORD:
            return "double word score";
        case SCRABBLE_BOARD_TRIPLE_WORD:
            return "triple word score";
        default:
            return "normal square";
    }
}

static void update_accessible_label(
    ScrabbleBoardViewCell *view_cell,
    ScrabbleBoardPremium premium,
    ScrabbleBoardCell cell) {
    char description[128];
    char column = (char)('A' + view_cell->position.column);

    if (cell.letter == '\0') {
        g_snprintf(
            description,
            sizeof(description),
            "Row %zu, column %c, %s, empty",
            view_cell->position.row + 1,
            column,
            premium_description(premium));
    } else if (cell.is_blank) {
        g_snprintf(
            description,
            sizeof(description),
            "Row %zu, column %c, blank tile representing %c, zero points",
            view_cell->position.row + 1,
            column,
            cell.letter);
    } else {
        char word[2] = {cell.letter, '\0'};
        int points = scrabble_score_word(word);

        g_snprintf(
            description,
            sizeof(description),
            "Row %zu, column %c, tile %c, %d %s",
            view_cell->position.row + 1,
            column,
            cell.letter,
            points,
            points == 1 ? "point" : "points");
    }

    gtk_accessible_update_property(
        GTK_ACCESSIBLE(view_cell->button),
        GTK_ACCESSIBLE_PROPERTY_LABEL,
        description,
        -1);
    gtk_widget_set_tooltip_text(view_cell->button, description);
}

static void update_cell(
    ScrabbleBoardViewCell *view_cell,
    ScrabbleBoardCell cell) {
    ScrabbleBoardPremium premium = SCRABBLE_BOARD_PREMIUM_NONE;
    const char *primary_text;
    char point_text[8] = "";

    (void)scrabble_board_get_premium(view_cell->position, &premium);
    gtk_widget_remove_css_class(view_cell->button, "board-tile-filled");
    gtk_widget_remove_css_class(view_cell->button, "board-blank-tile");

    if (cell.letter == '\0') {
        primary_text = premium_text(view_cell->position, premium);
    } else {
        char word[2] = {cell.letter, '\0'};
        int points = cell.is_blank ? 0 : scrabble_score_word(word);

        primary_text = word;
        g_snprintf(point_text, sizeof(point_text), "%d", points);
        gtk_widget_add_css_class(view_cell->button, "board-tile-filled");
        if (cell.is_blank) {
            gtk_widget_add_css_class(view_cell->button, "board-blank-tile");
        }
        gtk_label_set_text(
            GTK_LABEL(view_cell->primary_label), primary_text);
        gtk_label_set_text(
            GTK_LABEL(view_cell->secondary_label), point_text);
        update_accessible_label(view_cell, premium, cell);
        return;
    }

    gtk_label_set_text(GTK_LABEL(view_cell->primary_label), primary_text);
    gtk_label_set_text(GTK_LABEL(view_cell->secondary_label), "");
    update_accessible_label(view_cell, premium, cell);
}

static void select_position(
    ScrabbleBoardViewState *view,
    ScrabbleBoardPosition position,
    gboolean notify) {
    if (view->has_selection &&
        view->selected_position.row == position.row &&
        view->selected_position.column == position.column) {
        return;
    }

    if (view->has_selection) {
        gtk_widget_remove_css_class(
            view->cells[cell_index(view->selected_position)].button,
            "board-cell-selected");
    }

    view->selected_position = position;
    view->has_selection = TRUE;
    gtk_widget_add_css_class(
        view->cells[cell_index(position)].button,
        "board-cell-selected");

    if (notify && view->selection_callback != NULL) {
        view->selection_callback(
            view->widget, position, view->selection_data);
    }
}

static void on_cell_clicked(GtkButton *button, gpointer user_data) {
    ScrabbleBoardViewCell *view_cell = user_data;

    (void)button;
    select_position(view_cell->view, view_cell->position, TRUE);
}

static void destroy_board_view(gpointer data) {
    ScrabbleBoardViewState *view = data;

    if (view->selection_data_destroy != NULL) {
        view->selection_data_destroy(view->selection_data);
    }
    g_free(view);
}

static GtkWidget *create_axis_label(const char *text) {
    GtkWidget *label = gtk_label_new(text);

    gtk_widget_add_css_class(label, "board-axis-label");
    gtk_widget_set_size_request(label, BOARD_CELL_SIZE, BOARD_CELL_SIZE);
    return label;
}

GtkWidget *scrabble_board_view_new(void) {
    GtkWidget *board = gtk_grid_new();
    ScrabbleBoardViewState *view = g_new0(ScrabbleBoardViewState, 1);

    view->widget = board;
    gtk_grid_set_row_spacing(GTK_GRID(board), 2);
    gtk_grid_set_column_spacing(GTK_GRID(board), 2);
    gtk_widget_add_css_class(board, "scrabble-board");
    gtk_widget_set_halign(board, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(board, GTK_ALIGN_CENTER);
    gtk_accessible_update_property(
        GTK_ACCESSIBLE(board),
        GTK_ACCESSIBLE_PROPERTY_LABEL,
        "Scrabble board",
        -1);
    g_object_set_data_full(
        G_OBJECT(board),
        "scrabble-board-view",
        view,
        destroy_board_view);

    for (size_t column = 0; column < SCRABBLE_BOARD_SIZE; ++column) {
        char label_text[2] = {(char)('A' + column), '\0'};

        gtk_grid_attach(
            GTK_GRID(board),
            create_axis_label(label_text),
            (int)column + 1,
            0,
            1,
            1);
    }

    for (size_t row = 0; row < SCRABBLE_BOARD_SIZE; ++row) {
        char label_text[4];

        g_snprintf(label_text, sizeof(label_text), "%zu", row + 1);
        gtk_grid_attach(
            GTK_GRID(board),
            create_axis_label(label_text),
            0,
            (int)row + 1,
            1,
            1);

        for (size_t column = 0; column < SCRABBLE_BOARD_SIZE; ++column) {
            ScrabbleBoardPosition position = {row, column};
            ScrabbleBoardPremium premium = SCRABBLE_BOARD_PREMIUM_NONE;
            ScrabbleBoardViewCell *view_cell = &view->cells[cell_index(position)];
            GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
            ScrabbleBoardCell empty_cell = {'\0', 0};

            view_cell->view = view;
            view_cell->position = position;
            view_cell->button = gtk_button_new();
            view_cell->primary_label = gtk_label_new("");
            view_cell->secondary_label = gtk_label_new("");

            (void)scrabble_board_get_premium(position, &premium);
            gtk_widget_add_css_class(view_cell->button, "board-cell");
            gtk_widget_add_css_class(view_cell->button, premium_class(premium));
            gtk_widget_add_css_class(
                view_cell->primary_label, "board-cell-primary");
            gtk_widget_add_css_class(
                view_cell->secondary_label, "board-cell-points");
            gtk_widget_set_size_request(
                view_cell->button, BOARD_CELL_SIZE, BOARD_CELL_SIZE);
            gtk_widget_set_focus_on_click(view_cell->button, TRUE);
            gtk_box_append(GTK_BOX(content), view_cell->primary_label);
            gtk_box_append(GTK_BOX(content), view_cell->secondary_label);
            gtk_button_set_child(GTK_BUTTON(view_cell->button), content);
            gtk_grid_attach(
                GTK_GRID(board),
                view_cell->button,
                (int)column + 1,
                (int)row + 1,
                1,
                1);
            g_signal_connect(
                view_cell->button,
                "clicked",
                G_CALLBACK(on_cell_clicked),
                view_cell);
            update_cell(view_cell, empty_cell);
        }
    }

    return board;
}

void scrabble_board_view_set_board(
    GtkWidget *board_view,
    const ScrabbleBoard *board) {
    ScrabbleBoardViewState *view = board_view_state(board_view);

    g_return_if_fail(view != NULL);
    for (size_t row = 0; row < SCRABBLE_BOARD_SIZE; ++row) {
        for (size_t column = 0; column < SCRABBLE_BOARD_SIZE; ++column) {
            ScrabbleBoardPosition position = {row, column};
            ScrabbleBoardCell cell = {'\0', 0};

            if (board != NULL) {
                (void)scrabble_board_get_cell(board, position, &cell);
            }
            update_cell(&view->cells[cell_index(position)], cell);
        }
    }
}

void scrabble_board_view_select_position(
    GtkWidget *board_view,
    ScrabbleBoardPosition position) {
    ScrabbleBoardViewState *view = board_view_state(board_view);

    g_return_if_fail(view != NULL);
    g_return_if_fail(scrabble_board_position_is_valid(position));
    select_position(view, position, FALSE);
}

gboolean scrabble_board_view_get_selected_position(
    GtkWidget *board_view,
    ScrabbleBoardPosition *position) {
    ScrabbleBoardViewState *view = board_view_state(board_view);

    g_return_val_if_fail(view != NULL, FALSE);
    g_return_val_if_fail(position != NULL, FALSE);
    if (!view->has_selection) {
        return FALSE;
    }

    *position = view->selected_position;
    return TRUE;
}

void scrabble_board_view_clear_selection(GtkWidget *board_view) {
    ScrabbleBoardViewState *view = board_view_state(board_view);

    g_return_if_fail(view != NULL);
    if (view->has_selection) {
        gtk_widget_remove_css_class(
            view->cells[cell_index(view->selected_position)].button,
            "board-cell-selected");
        view->has_selection = FALSE;
    }
}

void scrabble_board_view_set_selection_callback(
    GtkWidget *board_view,
    ScrabbleBoardViewSelectionCallback callback,
    gpointer user_data,
    GDestroyNotify destroy_notify) {
    ScrabbleBoardViewState *view = board_view_state(board_view);

    g_return_if_fail(view != NULL);
    if (view->selection_data_destroy != NULL) {
        view->selection_data_destroy(view->selection_data);
    }
    view->selection_callback = callback;
    view->selection_data = user_data;
    view->selection_data_destroy = destroy_notify;
}
