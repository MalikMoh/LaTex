#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Function prototypes
void on_text_changed(GtkTextBuffer *buffer, gpointer data);
void compile_latex(const char *latex_code, const char *output_image);

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *hbox;
    GtkWidget *text_view;
    GtkWidget *image;
    GtkWidget *scroll_win;
    GtkTextBuffer *text_buffer;

    gtk_init(&argc, &argv);

    // Create main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "LaTeX Editor");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create horizontal box
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(window), hbox);

    // Create text view for LaTeX input
    text_view = gtk_text_view_new();
    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    g_signal_connect(text_buffer, "changed", G_CALLBACK(on_text_changed), image);

    // Add text view to a scrollable window
    scroll_win = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll_win), text_view);
    gtk_box_pack_start(GTK_BOX(hbox), scroll_win, TRUE, TRUE, 0);

    // Create image widget for output
    image = gtk_image_new();
    gtk_box_pack_start(GTK_BOX(hbox), image, TRUE, TRUE, 0);

    // Show all widgets
    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}

void on_text_changed(GtkTextBuffer *buffer, gpointer data) {
    GtkWidget *image = GTK_WIDGET(data);
    GtkTextIter start, end;
    gchar *text;

    gtk_text_buffer_get_bounds(buffer, &start, &end);
    text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    // Compile the LaTeX code and generate output.png
    compile_latex(text, "output.png");

    // Load the image into the image widget
    gtk_image_set_from_file(GTK_IMAGE(image), "output.png");

    g_free(text);
}

void compile_latex(const char *latex_code, const char *output_image) {
    FILE *file = fopen("temp.tex", "w");
    if (!file) return;

    // Write the LaTeX code to a temporary file
    fprintf(file, "\\documentclass{standalone}\n");
    fprintf(file, "\\usepackage{amsmath}\n");
    fprintf(file, "\\begin{document}\n");
    fprintf(file, "%s\n", latex_code);
    fprintf(file, "\\end{document}\n");
    fclose(file);

    // Compile the LaTeX code
    system("pdflatex -interaction=nonstopmode temp.tex");

    // Convert PDF to PNG
    system("pdftoppm -png temp.pdf > output.png");
}
