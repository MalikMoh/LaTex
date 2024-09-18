#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function prototypes
void displayEditor();
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
                printf("Enter LaTeX expression: ");
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = 0; // Remove newline
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
