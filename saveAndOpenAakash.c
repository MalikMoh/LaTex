#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to save content to a file
void saveToFile(const char *filename, const char *content) {
    FILE *file = fopen(filename, "w");  // Make sure to include stdio.h for FILE
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    fprintf(file, "%s", content);
    fclose(file);
}

// Function to open and read content from a file
void openFromFile(const char *filename, char *content, size_t maxSize) {
    FILE *file = fopen(filename, "r");  // Ensure FILE is defined in stdio.h
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    fread(content, 1, maxSize, file);
    content[maxSize - 1] = '\0'; // Ensure null termination
    fclose(file);
}

int main() {
    char content[1024];
    
    // Example usage of the functions
    saveToFile("example.txt", "This is a test content.");
    openFromFile("example.txt", content, sizeof(content));
    
    printf("File content: %s\n", content);
    return 0;
}
