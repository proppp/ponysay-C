
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>

#define PONIES_DIRECTORY "/usr/share/ponysay/ponies/"
#define MAX_LINE_LENGTH 10000
#define BUFFER_SIZE 10000 // To store the piped input

// Function to check if a path is a regular file
int isRegularFile(const char *path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

// Function to get a random pony file
char *getRandomPonyFile() {
    DIR *dir = opendir(PONIES_DIRECTORY);
    if (!dir) {
        perror("Error opening ponies directory");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    char *ponyFiles[1000]; // Assuming a max of 100 pony files
    int ponyCount = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // Only regular files
            ponyFiles[ponyCount++] = strdup(entry->d_name);
        }
    }
    closedir(dir);

    if (ponyCount == 0) {
        fprintf(stderr, "No pony files found\n");
        exit(EXIT_FAILURE);
    }

    // Seed random number generator and select a random pony
    srand(time(NULL));
    int randomIndex = rand() % ponyCount;
    return ponyFiles[randomIndex]; // Return the selected pony file
}

// Function to read piped input from stdin
char* readPipedInput() {
    char *input = malloc(BUFFER_SIZE);
    if (!input) {
        perror("Failed to allocate memory for input");
        exit(EXIT_FAILURE);
    }

    int totalBytes = 0;

    // Read the input until EOF
    while (fgets(input + totalBytes, BUFFER_SIZE - totalBytes, stdin)) {
        totalBytes += strlen(input + totalBytes);

        // Check if we need to reallocate for more input
        if (totalBytes >= BUFFER_SIZE - 1) {
            input = realloc(input, totalBytes + BUFFER_SIZE);
            if (!input) {
                perror("Failed to reallocate memory for input");
                exit(EXIT_FAILURE);
            }
        }
    }

    // Null-terminate the input string
    input[totalBytes] = '\0';
    return input;
}

// Function to print the contents of a pony file, replacing sections with piped input
void printPony(const char *ponyFileName, const char *input) {
    char path[MAX_LINE_LENGTH];
    snprintf(path, sizeof(path), "%s/%s", PONIES_DIRECTORY, ponyFileName);

    FILE *file = fopen(path, "r");
    if (!file) {
        perror("Error opening pony file");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    int inDollarSection = 0; // Flag to track if we're inside a "$$$" section
    char output[MAX_LINE_LENGTH * 10]; // Buffer to hold the final output
    output[0] = '\0'; // Initialize output buffer

    while (fgets(line, sizeof(line), file)) {
        char *startDollar = strstr(line, "$$$"); // Find start of "$$$"
        while (startDollar != NULL) {
            // Toggle `inDollarSection`
            inDollarSection = !inDollarSection;

            // Remove the `$$$` marker and move the pointer forward
            memmove(startDollar, startDollar + 3, strlen(startDollar + 3) + 1);

            // Search again for any more `$$$` in the same line
            startDollar = strstr(line, "$$$");
        }

        // Check for the balloon placeholder
        char *balloonPlaceholder = strstr(line, "$balloon");
        if (balloonPlaceholder) {
            // Replace the placeholder with the piped input
            strcat(output, input);  // Append the piped input to output
        } else {
            // Append line to output if we're not inside a "$$$" section
            if (!inDollarSection) {
                strcat(output, line);
            }
        }
    }
    fclose(file);

    // Remove dollar signs from the final output
    for (int i = 0; output[i] != '\0'; i++) {
        if (output[i] != '$') {
            putchar(output[i]); // Print each character except '$'
        }
    }
}

int main() {
    char *randomPonyFile = getRandomPonyFile(); // Get a random pony file
    char *pipedInput = readPipedInput(); // Read input from stdin

    printPony(randomPonyFile, pipedInput); // Print the pony content with the piped input

    free(randomPonyFile); // Free the dynamically allocated string
    free(pipedInput);     // Free the dynamically allocated input
    return 0;
}

