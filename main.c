#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function prototypes
void displayEditor(char *input, size_t maxSize);
void parseInput(const char *input, char *output);
int checkSyntax(const char *input);
void saveToFile(const char *filename, const char *content);
void openFromFile(const char *filename, char *content, size_t maxSize);

int main() {
    char input[1024];
    char output[1024];
    char filename[100];
    int option;

    while (1) {
        printf("\nLaTeX Tool\n");
        printf("1. Open File\n");
        printf("2. Edit LaTeX\n");
        printf("3. Save File\n");
        printf("4. Exit\n");
        printf("Choose an option: ");
        scanf("%d", &option);
        getchar(); // Consume the newline

        switch (option) {
            case 1: // Open file
                printf("Enter filename to open: ");
                fgets(filename, 100, stdin);
                filename[strcspn(filename, "\n")] = 0; // Remove newline
                openFromFile(filename, input, sizeof(input));
                printf("File content loaded:\n%s\n", input);
                break;
            case 2: // Edit LaTeX
                displayEditor(input, sizeof(input)); // Use displayEditor to get user input
                if (checkSyntax(input)) {
                    parseInput(input, output);
                    printf("Converted Output: %s\n", output);
                } else {
                    printf("Syntax Error in LaTeX expression.\n");
                }
                break;
            case 3: // Save file
                printf("Enter filename to save: ");
                fgets(filename, 100, stdin);
                filename[strcspn(filename, "\n")] = 0; // Remove newline
                saveToFile(filename, input);
                printf("File saved successfully.\n");
                break;
            case 4: // Exit
                printf("Exiting...\n");
                exit(0);
            default:
                printf("Invalid option, please try again.\n");
        }
    }
    return 0;
}

void displayEditor(char *input, size_t maxSize) {
    // Simple editor logic: get user input for LaTeX
    printf("Enter LaTeX expression (max %zu characters):\n", maxSize - 1);
    fgets(input, maxSize, stdin);
    input[strcspn(input, "\n")] = 0; // Remove newline character
}

void parseInput(const char *input, char *output) {
    // Simple parsing logic for LaTeX expressions
    // This is a simplified version; a real implementation would be more complex
    strcpy(output, ""); // Clear output

    while (*input) {
        if (*input == '\\') {
            input++;
            if (strncmp(input, "frac", 4) == 0) {
                strcat(output, "fraction");
                input += 4;
            } else if (strncmp(input, "sqrt", 4) == 0) {
                strcat(output, "root");
                input += 4;
            } else if (strncmp(input, "int", 3) == 0) {
                strcat(output, "integral");
                input += 3;
            } else {
                strcat(output, "unknown_command");
            }
        } else if (*input == '$') {
            // Ignore $
            input++;
        } else {
            strncat(output, input, 1);
            input++;
        }
    }
}

int checkSyntax(const char *input) {
    // Basic syntax checking (real implementation would need more rules)
    int openBraces = 0;

    while (*input) {
        if (*input == '{') openBraces++;
        if (*input == '}') openBraces--;
        if (openBraces < 0) return 0; // More closing braces than opening
        input++;
    }
    return openBraces == 0; // Return 1 if all braces match
}

void saveToFile(const char *filename, const char *content) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    fprintf(file, "%s", content);
    fclose(file);
}

void openFromFile(const char *filename, char *content, size_t maxSize) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    fread(content, 1, maxSize, file);
    content[maxSize - 1] = '\0'; // Ensure null termination
    fclose(file);
}
