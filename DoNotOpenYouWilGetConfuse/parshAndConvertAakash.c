#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_HTML_SIZE 8192
#define MAX_LATEX_SIZE 2048

// Function to generate LaTeX content to be saved and compiled to PDF
void save_as_pdf(const char *latex_code, const char *filename) {
    // Temporary .tex file
    char temp_tex_file[256];
    snprintf(temp_tex_file, sizeof(temp_tex_file), "%s.tex", filename);

    // Write LaTeX content to the .tex file
    FILE *file = fopen(temp_tex_file, "w");
    if (file != NULL) {
        fprintf(file, "\\documentclass{article}\n"
                      "\\usepackage{amsmath}\n"
                      "\\usepackage{amsfonts}\n"
                      "\\usepackage{amssymb}\n"
                      "\\usepackage{graphicx}\n"
                      "\\begin{document}\n"
                      "\\begin{center}\n"
                      "\\[%s\\]\n"
                      "\\end{center}\n"
                      "\\end{document}", latex_code);
        fclose(file);

        // Run pdflatex to generate the PDF
        char command[512];
        snprintf(command, sizeof(command), "pdflatex -interaction=nonstopmode -output-directory=%s %s",
                 ".", temp_tex_file);
        system(command);  // This command will run pdflatex to generate the PDF
    }
}

// Callback function to handle "Save as PDF" action
void on_save_as_pdf_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkWindow *parent_window = GTK_WINDOW(data);

    // Create a file chooser dialog for saving files
    dialog = gtk_file_chooser_dialog_new("Save as PDF",
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

        // Save the content as a PDF
        save_as_pdf(text, filename);

        g_free(text);
        g_free(filename);
    }

    // Destroy the dialog
    gtk_widget_destroy(dialog);
}

// Callback function to generate HTML content for LaTeX rendering
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

    // Create text view for editor
    GtkWidget *text_view = gtk_text_view_new();
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

    // Create "Save as PDF" button and connect it to the save as PDF callback
    GtkWidget *save_as_pdf_button = gtk_button_new_with_label("Save as PDF");
    g_signal_connect(G_OBJECT(save_as_pdf_button), "clicked", G_CALLBACK(on_save_as_pdf_button_clicked), window);
    gtk_box_pack_start(GTK_BOX(button_box), save_as_pdf_button, FALSE, FALSE, 0);

    // Show all widgets
    gtk_widget_show_all(window);

    // Enter the GTK main loop
    gtk_main();

    return 0;
}
