#include <bits/stdc++.h>
using namespace std;

//global variables for file handling and state tracking
FILE* inFile  = NULL;
FILE* outFile = NULL;

bool inMultiLineComment  = false;     
bool inSingleLineComment = false;
bool inString            = false;
bool lastCharWasSpace    = false;

// Function prototypes
bool initializeFiles(const char* inputSource, const char* outputSource);
void runCoreProcessingEngine();
void displayFileContent(const char* filename, const char* label);

// Handlers for specific states
void handleMultiLineComment(char ch);
void handleSingleLineComment(char ch);
void handleStringLiteral(char ch);

// Main function
int main() {
    if (!initializeFiles("input.cpp", "output.cpp")) {
        return 1; 
    }

    // Run the core processing engine to filter comments and whitespace
    runCoreProcessingEngine();

    // Display the contents of the original and filtered files for verification
    displayFileContent("input.cpp", "ORIGINAL C++ INPUT FILE");
    displayFileContent("output.cpp", "FILTERED C++ OUTPUT FILE");

    return 0;
}

// Function to initialize input and output files
bool initializeFiles(const char* inputSource, const char* outputSource) {
    inFile = fopen(inputSource, "r");
    outFile = fopen(outputSource, "w");

    if (inFile == NULL) {
        cerr << "Error: Could not open existing input file: " << inputSource << endl;
        return false;
    }
    if (outFile == NULL) {
        cerr << "Error: Could not create output file: " << outputSource << endl;
        fclose(inFile);
        return false;
    }
    return true; 
}

// Core processing engine to read input file, filter comments and whitespace, and write to output file
void runCoreProcessingEngine() {
    int c;
    while ((c = fgetc(inFile)) != EOF) {
        char ch = (char)c;

        // 1. Delegate to specific handlers if flags are already raised
        if (inMultiLineComment) {
            handleMultiLineComment(ch);
        } 
        else if (inSingleLineComment) {
            handleSingleLineComment(ch);
        } 
        else if (inString) {
            handleStringLiteral(ch);
        } 
        // 2. Baseline Behavior: Process as Active Code directly inside the loop
        else {
            if (ch == '"') {
                inString = true;
                fputc(ch, outFile);
                lastCharWasSpace = false;
                continue;
            }
            if (ch == '/') {
                char nextCh = fgetc(inFile);
                if (nextCh == '/') {
                    inSingleLineComment = true;
                    continue;
                } 
                else if (nextCh == '*') {
                    inMultiLineComment = true; 
                    continue;
                } 
                else {
                    ungetc(nextCh, inFile); 
                }
            }
            if (isspace(ch)) {
                if (!lastCharWasSpace) {
                    fputc(' ', outFile);
                    lastCharWasSpace = true;
                }
            } else {
                fputc(ch, outFile);
                lastCharWasSpace = false;
            }
        }
    }
    fclose(inFile);
    fclose(outFile);
}

// Handler for multi-line comments
void handleMultiLineComment(char ch) {
    if (ch == '*') {
        char nextCh = fgetc(inFile);
        if (nextCh == '/') {
            inMultiLineComment = false; 
            
            if (!lastCharWasSpace) {
                fputc(' ', outFile);
                lastCharWasSpace = true;
            }
            return;
        } else {
            ungetc(nextCh, inFile);
        }
    }
}

// Handler for single-line comments
void handleSingleLineComment(char ch) {
    if (ch == '\n') {
        inSingleLineComment = false;
        lastCharWasSpace = false; 
    }
}

// Handler for string literals
void handleStringLiteral(char ch) {
    if (ch == '\\') { 
        fputc(ch, outFile);
        int nextC = fgetc(inFile);
        if (nextC != EOF) {
            fputc((char)nextC, outFile);
        }
        return;
    }
    if (ch == '"') {
        inString = false; 
    }
    fputc(ch, outFile);
    lastCharWasSpace = false;
}

// Function to display the contents of a file with a label
void displayFileContent(const char* filename, const char* label) {
    FILE* file = fopen(filename, "r");
    
    cout << "\n==================DISPLAYING=======================\n";
    cout << "  " << label << " (" << filename << ")\n";
    cout << "=====================================================\n";
    
    if (file == NULL) {
        cout << "Error opening file for display.\n";
        return;
    }
    
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        cout << buffer;
    }
    cout << "\n===================X======================\n";
    fclose(file);
}