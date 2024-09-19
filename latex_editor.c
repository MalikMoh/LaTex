//how to compile
//make sure you have all required library installed 
//gcc $(pkg-config --cflags gtk+-3.0 webkit2gtk-4.0) -o latex_editor latex_editor.c $(pkg-config --libs gtk+-3.0 webkit2gtk-4.0)
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <stdbool.h>
#include <string.h>

#include <string.h>

// Increase buffer size to handle larger HTML content
#define MAX_HTML_SIZE 8192
#define MAX_LATEX_SIZE 2048

// Function to strip $ signs and generate HTML with embedded MathJax for LaTeX rendering
char* generate_html(const char *latex) {
    static char html_template[MAX_HTML_SIZE];

    // Create a copy of the latex input to modify, limiting its size
    char clean_latex[MAX_LATEX_SIZE];
    strncpy(clean_latex, latex, MAX_LATEX_SIZE - 1); // Leave space for null terminator
    clean_latex[MAX_LATEX_SIZE - 1] = '\0'; // Ensure null termination

    // Remove leading and trailing $ if present
    size_t len = strlen(clean_latex);
    if (clean_latex[0] == '$' && clean_latex[len - 1] == '$') {
        clean_latex[len - 1] = '\0'; // Remove trailing $
        memmove(clean_latex, clean_latex + 1, len - 1); // Remove leading $
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
        "<div id=\"math\">\\[%s\\]</div>"
        "</body>"
        "</html>", clean_latex);
    
    return html_template;
}


// Function to check basic syntax for common LaTeX errors
bool check_latex_syntax(const char *text) {
    int balance = 0;
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] == '{') balance++;
        if (text[i] == '}') balance--;
        if (balance < 0) return false; // Misbalanced braces
    }
    return (balance == 0); // Ensure all opened braces are closed
}

// Function to handle "Open" and "Save" menu items
void on_file_open(GtkWidget *widget, gpointer user_data) {
    GtkWidget *dialog;
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);

    dialog = gtk_file_chooser_dialog_new("Open File",
                                         NULL,
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                         "_Open", GTK_RESPONSE_ACCEPT,
                                         NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename;
        char *content;
        gsize length;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);

        if (g_file_get_contents(filename, &content, &length, NULL)) {
            gtk_text_buffer_set_text(buffer, content, length);
            g_free(content);
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

void on_file_save(GtkWidget *widget, gpointer user_data) {
    GtkWidget *dialog;
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);

    dialog = gtk_file_chooser_dialog_new("Save File",
                                         NULL,
                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                         "_Save", GTK_RESPONSE_ACCEPT,
                                         NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);

        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        char *content = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
        g_file_set_contents(filename, content, -1, NULL);
        g_free(content);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

// Callback when text in the editor is changed
void on_text_changed(GtkTextBuffer *buffer, gpointer user_data) {
    GtkWidget *web_view = GTK_WIDGET(user_data);

    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    char *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    // Check for syntax errors
    if (!check_latex_syntax(text)) {
        webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), "<b>Error: Syntax Error in LaTeX</b>", NULL);
    } else {
        // Generate HTML with MathJax
        char *html_content = generate_html(text);
        // Load the HTML content into the web view
        webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), html_content, NULL);
    }

    g_free(text);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "LaTeX Editor");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Create a horizontal box to hold the widgets (vertical partition)
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_container_add(GTK_CONTAINER(window), hbox);

    // Create a text view for the editor
    GtkWidget *text_view = gtk_text_view_new();
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_box_pack_start(GTK_BOX(hbox), text_view, TRUE, TRUE, 2);

    // Create a web view for the output
    GtkWidget *web_view = webkit_web_view_new();
    gtk_box_pack_start(GTK_BOX(hbox), web_view, TRUE, TRUE, 2);

    // Menu for File Open and Save
    GtkWidget *menubar = gtk_menu_bar_new();
    GtkWidget *fileMenu = gtk_menu_new();
    GtkWidget *fileMi = gtk_menu_item_new_with_label("File");
    GtkWidget *openMi = gtk_menu_item_new_with_label("Open");
    GtkWidget *saveMi = gtk_menu_item_new_with_label("Save");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileMi), fileMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), openMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), saveMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileMi);

    // Layout for menu bar
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Connect signals for open and save
    g_signal_connect(G_OBJECT(openMi), "activate", G_CALLBACK(on_file_open), buffer);
    g_signal_connect(G_OBJECT(saveMi), "activate", G_CALLBACK(on_file_save), buffer);

    // Connect the text buffer's "changed" signal to update the web view
    g_signal_connect(buffer, "changed", G_CALLBACK(on_text_changed), web_view);

    // Connect the destroy signal to exit the GTK main loop
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Show all widgets
    gtk_widget_show_all(window);

    // Enter the GTK main loop
    gtk_main();

    return 0;
}
