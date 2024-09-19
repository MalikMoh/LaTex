#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <string.h>
#include <stdio.h>

#define MAX_HTML_SIZE 8192
#define MAX_LATEX_SIZE 2048

// Function to generate HTML content for LaTeX rendering
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h> // Include this for isalpha
#include <regex.h> // You may not need this unless you're using regex functions

#define MAX_HTML_SIZE 8192
#define MAX_LATEX_SIZE 2048

// Function to generate HTML content for LaTeX rendering
// Function to generate HTML content for LaTeX rendering
char* generate_html(const char *latex) {
    static char html_template[MAX_HTML_SIZE];

    // Create a copy of the latex input to modify, limiting its size
    char clean_latex[MAX_LATEX_SIZE];
    strncpy(clean_latex, latex, MAX_LATEX_SIZE - 1); // Leave space for null terminator
    clean_latex[MAX_LATEX_SIZE - 1] = '\0'; // Ensure null termination

    // Output buffer to store modified LaTeX
    char modified_latex[MAX_LATEX_SIZE] = "";
    const char *ptr = clean_latex;
    char temp[MAX_LATEX_SIZE];
    int in_latex_mode = 0;  // Flag to track if we are inside LaTeX math mode

    // Iterate through the input string
    while (*ptr != '\0') {
        if (*ptr == '$') {
            // Toggle math mode when encountering $
            in_latex_mode = !in_latex_mode;

            if (in_latex_mode) {
                // Entering LaTeX mode, add opening LaTeX math delimiters
                strcat(modified_latex, "\\(");
            } else {
                // Exiting LaTeX mode, add closing LaTeX math delimiters
                strcat(modified_latex, "\\)");
            }

            ptr++;  // Move past the $
        } else if (in_latex_mode && *(ptr + 1) == '/' && isalpha(*ptr) && isalpha(*(ptr + 2))) {
            // If pattern is like a/b inside LaTeX mode, convert it to \frac{a}{b}
            snprintf(temp, sizeof(temp), "\\frac{%c}{%c}", *ptr, *(ptr + 2));
            strcat(modified_latex, temp);
            ptr += 3; // Skip over a/b
        } else {
            // Copy regular characters, including text outside LaTeX mode
            strncat(modified_latex, ptr, 1);
            ptr++;
        }
    }

    // Generate the HTML content with MathJax
    snprintf(html_template, sizeof(html_template),
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<script type=\"text/javascript\" async"
        " src=\"https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js\">"
        "</script>"
        "<style>"
        "body { font-size: 20px; }"
        "</style>"
        "</head>"
        "<body>"
        "<div id=\"math\">%s</div>"
        "</body>"
        "</html>", modified_latex);
    
    return html_template;
}




// Callback function to handle "Open" action
void on_open_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkWindow *parent_window = GTK_WINDOW(data);

    // Create a file chooser dialog for opening files
    dialog = gtk_file_chooser_dialog_new("Open File",
                                         parent_window,
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                         "_Open", GTK_RESPONSE_ACCEPT,
                                         NULL);

    // Show the dialog and wait for a response
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);

        // Read the content of the selected file
        FILE *file = fopen(filename, "r");
        if (file != NULL) {
            GtkWidget *text_view = g_object_get_data(G_OBJECT(parent_window), "text_view");
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
            char file_content[MAX_LATEX_SIZE];
            size_t len = fread(file_content, 1, sizeof(file_content) - 1, file);
            file_content[len] = '\0'; // Null-terminate the string
            fclose(file);

            // Load the content into the editor
            gtk_text_buffer_set_text(buffer, file_content, -1);
        }

        g_free(filename);
    }

    // Destroy the dialog
    gtk_widget_destroy(dialog);
}

// Callback function to handle "Save" action
void on_save_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkWindow *parent_window = GTK_WINDOW(data);

    // Create a file chooser dialog for saving files
    dialog = gtk_file_chooser_dialog_new("Save File",
                                         parent_window,
                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                         "_Save", GTK_RESPONSE_ACCEPT,
                                         NULL);

    // Show the dialog and wait for a response
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);

        // Retrieve the current content from the editor
        GtkWidget *text_view = g_object_get_data(G_OBJECT(parent_window), "text_view");
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        char *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

        // Write the content to the file
        FILE *file = fopen(filename, "w");
        if (file != NULL) {
            fputs(text, file);
            fclose(file);
        }

        g_free(text);
        g_free(filename);
    }

    // Destroy the dialog
    gtk_widget_destroy(dialog);
}

// Callback function for text changes in the editor
void on_text_changed(GtkTextBuffer *buffer, gpointer data) {
    WebKitWebView *web_view = WEBKIT_WEB_VIEW(data);

    // Get the text from the buffer
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    char *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    // Generate HTML and load it into the web view
    char *html_content = generate_html(text);
    webkit_web_view_load_html(web_view, html_content, NULL);

    g_free(text);
}

int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "LaTeX Editor");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Create vertical box layout
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create horizontal box layout
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

    // Create text view for editor and align it to top-left
    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    gtk_box_pack_start(GTK_BOX(hbox), text_view, TRUE, TRUE, 0);

    // Create a WebKit WebView for output
    GtkWidget *web_view = webkit_web_view_new();
    gtk_box_pack_start(GTK_BOX(hbox), web_view, TRUE, TRUE, 0);

    // Store the text view and web view in the window's user data
    g_object_set_data(G_OBJECT(window), "text_view", text_view);
    g_object_set_data(G_OBJECT(window), "web_view", web_view);

    // Connect the text buffer's "changed" signal to update the web view
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    g_signal_connect(buffer, "changed", G_CALLBACK(on_text_changed), web_view);

    // Create a horizontal box for buttons
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 0);

    // Create "Open" button and connect it to the open callback
    GtkWidget *open_button = gtk_button_new_with_label("Open");
    g_signal_connect(G_OBJECT(open_button), "clicked", G_CALLBACK(on_open_button_clicked), window);
    gtk_box_pack_start(GTK_BOX(button_box), open_button, FALSE, FALSE, 0);

    // Create "Save" button and connect it to the save callback
    GtkWidget *save_button = gtk_button_new_with_label("Save");
    g_signal_connect(G_OBJECT(save_button), "clicked", G_CALLBACK(on_save_button_clicked), window);
    gtk_box_pack_start(GTK_BOX(button_box), save_button, FALSE, FALSE, 0);

    // Show all widgets
    gtk_widget_show_all(window);

    // Enter the GTK main loop
    gtk_main();

    return 0;
}
