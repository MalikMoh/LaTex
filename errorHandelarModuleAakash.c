
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
