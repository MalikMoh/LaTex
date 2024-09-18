#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Function prototypes
void on_text_changed(GtkTextBuffer *buffer, gpointer data);
void parse_latex(const char *input, char *output, size_t max_output_size);
const char *to_superscript(char c);

// Function to convert a single-digit number to a superscript character (as a string)
const char *to_superscript(char c) {
    switch (c) {
        case '0': return "\u2070";
        case '1': return "\u00B9";
        case '2': return "\u00B2";
        case '3': return "\u00B3";
        case '4': return "\u2074";
        case '5': return "\u2075";
        case '6': return "\u2076";
        case '7': return "\u2077";
        case '8': return "\u2078";
        case '9': return "\u2079";
        default: return "";
    }
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *hbox;
    GtkWidget *text_view;
    GtkWidget *output_view;
    GtkWidget *scroll_win_left;
    GtkWidget *scroll_win_right;
    GtkTextBuffer *text_buffer;
    GtkTextBuffer *output_buffer;

    gtk_init(&argc, &argv);

    // Create main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "LaTeX Editor");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create horizontal box
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(window), hbox);

    // Create text view for LaTeX input (Left Side)
    text_view = gtk_text_view_new();
    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    scroll_win_left = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll_win_left), text_view);
    gtk_box_pack_start(GTK_BOX(hbox), scroll_win_left, TRUE, TRUE, 0);

    // Create text view for output display (Right Side)
    output_view = gtk_text_view_new();
    output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_view));
    gtk_text_view_set_editable(GTK_TEXT_VIEW(output_view), FALSE); // Make this view read-only
    scroll_win_right = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll_win_right), output_view);
    gtk_box_pack_start(GTK_BOX(hbox), scroll_win_right, TRUE, TRUE, 0);

    // Connect the buffer change event to our function
    g_signal_connect(text_buffer, "changed", G_CALLBACK(on_text_changed), output_buffer);

    // Show all widgets
    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}

void on_text_changed(GtkTextBuffer *buffer, gpointer data) {
    GtkTextBuffer *output_buffer = GTK_TEXT_BUFFER(data);
    GtkTextIter start, end;
    gchar *input_text;
    char output_text[1024]; // Buffer to store parsed LaTeX

    gtk_text_buffer_get_bounds(buffer, &start, &end);
    input_text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    // Parse the LaTeX input and generate output
    parse_latex(input_text, output_text, sizeof(output_text));

    // Set the output buffer with the parsed text
    gtk_text_buffer_set_text(output_buffer, output_text, -1);

    g_free(input_text);
}

void parse_latex(const char *input, char *output, size_t max_output_size) {
    // Initialize the output buffer
    strcpy(output, "");

    // Simple parsing logic for some LaTeX commands
    while (*input) {
        if (*input == '\\') {
            input++;
            if (strncmp(input, "frac", 4) == 0) {
                strncat(output, "Fraction: ", max_output_size - strlen(output) - 1);
                input += 4;
            } else if (strncmp(input, "sqrt", 4) == 0) {
                strncat(output, "√", max_output_size - strlen(output) - 1);
                input += 4;
                while (*input == '{' || *input == '}') {
                    input++;
                }
            } else if (strncmp(input, "sum", 3) == 0) {
                strncat(output, "Summation: ", max_output_size - strlen(output) - 1);
                input += 3;
            } else {
                strncat(output, "Unknown Command: ", max_output_size - strlen(output) - 1);
            }
        } else if (*input == '^') {
            input++;
            if (isdigit(*input)) {
                const char *sup = to_superscript(*input);
                strncat(output, sup, max_output_size - strlen(output) - 1);
                input++;
            }
        } else if (*input == '$') {
            // Ignore the $ character for simplicity
            input++;
        } else {
            strncat(output, input, 1); // Append the character
            input++;
        }
    }
}
