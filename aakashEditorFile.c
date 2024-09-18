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
