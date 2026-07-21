#include <bits/stdc++.h>
using namespace std;

// ============================================================================
// GLOBAL VARIABLES & FILE NAMES
// ============================================================================
const char* inputFileName  = "output.cpp";   // Source code from Assignment 1
const char* step1FileName  = "output1.txt";  // Intermediate file for space-separated lexemes
const char* step2FileName  = "output2.txt";  // Final tokenized output file

FILE* inFile  = NULL;
FILE* outFile = NULL;

bool lastCharWasSpace = false;
bool inString         = false;

// Set of C/C++ Keywords
unordered_set<string> keywords = {
    "char", "int", "float", "double", "if", "else", "while", "for", 
    "return", "void", "string", "cout", "cin", "using", "namespace", "#include"
};

// Set of Operators
unordered_set<string> operators = {
    "=", "+", "-", "*", "/", "%", "==", "!=", "<", ">", "<=", ">=", 
    "+=", "-=", "*=", "/=", "++", "--", "<<", ">>"
};

// Set of Parentheses
unordered_set<string> parentheses = {
    "(", ")", "{", "}", "[", "]"
};

// Set of Separators
unordered_set<string> separators = {
    ";", ",", "\'", "\""
};

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================
// Step 1 Helper Functions
bool initializeFiles(const char* inPath, const char* outPath);
void closeFiles();
bool isSymbol(char ch);
bool isCompoundOperator(char ch, char nextCh);
void writeSpace();
void writeChar(char ch);
void handleSymbol(char ch);
void handleRegularCode(char ch);
void processLexemeSeparation();

// Step 2 DFA & Tokenizer Functions
bool isIdentifierDFA(const string& token);
bool isNumberDFA(const string& token);
string categorizeToken(const string& token);
void processTokenization();
bool isCompleteStringLiteral(const string& str);

// Helper for displaying files on console
void displayFileContent(const char* filename, const char* label);

// MAIN FUNCTION
int main() {
    // ------------------------------------------------------------------------
    // STEP 1: Lexeme Separation
    // ------------------------------------------------------------------------
    if (!initializeFiles(inputFileName, step1FileName)) {
        return 1;
    }

    cout << "Executing Step 1: Lexeme Separation..." << endl;
    processLexemeSeparation();
    closeFiles(); // Close files after Step 1 finishes
    cout << "Step 1 complete! Output saved to: " << step1FileName << endl;

    // ------------------------------------------------------------------------
    // STEP 2: Tokenization & DFA Execution
    // ------------------------------------------------------------------------
    cout << "Executing Step 2: Tokenization using DFAs..." << endl;
    processTokenization(); 
    cout << "Step 2 complete! Output saved to: " << step2FileName << endl;

    // ------------------------------------------------------------------------
    // DISPLAY RESULTS
    // ------------------------------------------------------------------------
    displayFileContent(inputFileName, "INPUT SOURCE FILE (Assignment 1)");
    displayFileContent(step1FileName, "STEP 1 OUTPUT: LEXEME SEPARATION");
    displayFileContent(step2FileName, "STEP 2 OUTPUT: TOKENIZED CATEGORIES");

    return 0;
}

// ============================================================================
// FILE MANAGEMENT MODULES
// ============================================================================
// Opens input and output files based on passed paths
bool initializeFiles(const char* inPath, const char* outPath) {
    inFile  = fopen(inPath, "r");
    outFile = fopen(outPath, "w");

    if (inFile == NULL) {
        cerr << "Error: Could not open file: " << inPath << endl;
        return false;
    }
    if (outFile == NULL) {
        cerr << "Error: Could not create file: " << outPath << endl;
        fclose(inFile);
        return false;
    }
    return true;
}
// Safely closes open file pointers
void closeFiles() {
    if (inFile != NULL)  fclose(inFile);
    if (outFile != NULL) fclose(outFile);
}

// ============================================================================
// STEP 1: LEXEME SEPARATION MODULES
// ============================================================================

// Checks if character is an operator or separator symbol
bool isSymbol(char ch) {
    string symbols = "=+-*/%<>&|^!;,(){}[]";
    return symbols.find(ch) != string::npos;
}

// Checks if two consecutive characters form a 2-character compound operator
bool isCompoundOperator(char ch, char nextCh) {
    return (ch == '=' && nextCh == '=') ||
           (ch == '<' && nextCh == '=') ||
           (ch == '>' && nextCh == '=') ||
           (ch == '!' && nextCh == '=') ||
           (ch == '+' && nextCh == '=') ||
           (ch == '-' && nextCh == '=') ||
           (ch == '*' && nextCh == '=') ||
           (ch == '/' && nextCh == '=') ||
           (ch == '<' && nextCh == '<') ||
           (ch == '>' && nextCh == '>');
}

// Safely writes a single space to output without duplicating existing spaces
void writeSpace() {
    if (!lastCharWasSpace) {
        fputc(' ', outFile);
        lastCharWasSpace = true;
    }
}

// Writes a character to output file and tracks space state
void writeChar(char ch) {
    fputc(ch, outFile);
    lastCharWasSpace = (ch == ' ');
}

// Handler for symbol tokens with look-ahead logic for compound operators
void handleSymbol(char ch) {
    int nextC = fgetc(inFile);
    char nextCh = (nextC != EOF) ? (char)nextC : '\0';

    if (nextC != EOF && isCompoundOperator(ch, nextCh)) {
        writeSpace();
        fputc(ch, outFile);
        fputc(nextCh, outFile);
        fputc(' ', outFile);
        lastCharWasSpace = true;
    } 
    else {
        if (nextC != EOF) {
            ungetc(nextC, inFile); // Return character if not part of 2-char operator
        }
        writeSpace();
        fputc(ch, outFile);
        fputc(' ', outFile);
        lastCharWasSpace = true;
    }
}

// Handler for regular code characters and spaces
void handleRegularCode(char ch) {
    if (isspace(ch)) {
        writeSpace();
    } else {
        writeChar(ch);
    }
}

// Main processing function for Step 1
void processLexemeSeparation() {
    int c;
    char prevCh = '\0'; // Track previous character to detect escapes like \"

    while ((c = fgetc(inFile)) != EOF) {
        char ch = (char)c;

        // Check for double quotes, but make sure it's NOT an escaped quote (\")
        if (ch == '"' && prevCh != '\\') {
            inString = !inString;
            writeChar(ch);
            prevCh = ch;
            continue;
        }

        // Inside string literals, preserve every character exactly as-is
        if (inString) {
            writeChar(ch);
            prevCh = ch;
            continue;
        }

        // Route normal code
        if (isSymbol(ch)) {
            handleSymbol(ch);
        } else {
            handleRegularCode(ch);
        }

        prevCh = ch; // Update previous character
    }
}

// ============================================================================
// STEP 2: DFA STATE MACHINES (MANDATORY REQUIREMENT)
// ============================================================================

/**
 * Identifier DFA (id)
 * State 0: Initial state
 * State 1: Valid Identifier state (Accepting)
 * State 2: Trap state (Rejecting)
 */
bool isIdentifierDFA(const string& token) {
    int state = 0;

    for (char ch : token) {
        if (state == 0) {
            if (isalpha(ch) || ch == '_') state = 1;
            else state = 2; // Dead state if starts with digit/symbol
        } 
        else if (state == 1) {
            if (isalnum(ch) || ch == '_') state = 1; // Self-loop
            else state = 2; // Dead state
        } 
        else if (state == 2) {
            state = 2; // Dead state self-loop
        }
    }

    return (state == 1);
}

/**
 * Numeric Constant DFA (num)
 * State 0: Initial state
 * State 1: Integer state (Accepting)
 * State 2: Decimal point seen (Expecting fraction digit)
 * State 3: Floating point state (Accepting)
 * State 4: Trap state (Rejecting - catches letters like 'o' in '100.o5')
 */
bool isNumberDFA(const string& token) {
    int state = 0;

    for (char ch : token) {
        if (state == 0) {
            if (isdigit(ch)) state = 1;      // e.g., "5" or "10"
            else if (ch == '.') state = 2;   // e.g., ".90" (transitions to waiting for fraction digit)
            else state = 4;                  // Trap state
        } 
        else if (state == 1) {
            if (isdigit(ch)) state = 1;      // Integer continuation
            else if (ch == '.') state = 2;   // e.g., "100."
            else state = 4;                  // Trap state
        } 
        else if (state == 2) {
            if (isdigit(ch)) state = 3;      // Fraction digit seen -> Valid Float State
            else state = 4;                  // Trap state (e.g., '.o' or '100.o')
        } 
        else if (state == 3) {
            if (isdigit(ch)) state = 3;      // Float continuation
            else state = 4;                  // Trap state (e.g., second dot '1.2.3')
        } 
        else if (state == 4) {
            state = 4;                       // Trap state self-loop
        }
    }

    // Accepting States: State 1 (Integer, e.g. "50") or State 3 (Float, e.g. "2.5" or ".90")
    return (state == 1 || state == 3);
}

// ============================================================================
// STEP 2: TOKEN CLASSIFICATION ENGINE
// ============================================================================
// Categorizes a single token string into [category lexeme]
string categorizeToken(const string& token) {
    // 0. Check for String Literals (starts and ends with quote)
    if (token.length() >= 2 && token.front() == '"' && token.back() == '"') {
        return "[str " + token + "]"; // Or [lit " + token + "]
    }
    // 1. Check Keywords
    if (keywords.find(token) != keywords.end()) {
        return "[kw " + token + "]";
    }
    // 2. Check Operators
    if (operators.find(token) != operators.end()) {
        return "[op " + token + "]";
    }
    // 3. Check Parentheses
    if (parentheses.find(token) != parentheses.end()) {
        return "[par " + token + "]";
    }
    // 4. Check Separators
    if (separators.find(token) != separators.end()) {
        return "[sep " + token + "]";
    }
    // 5. Check Identifier using DFA
    if (isIdentifierDFA(token)) {
        return "[id " + token + "]";
    }
    // 6. Check Number using DFA
    if (isNumberDFA(token)) {
        return "[num " + token + "]";
    }
    // 7. Fallback for Unknown Tokens (e.g., 100.o5)
    return "[unkn " + token + "]";
}
// Helper function to check if a string ends with a real closing quote (not an escaped \")
bool isCompleteStringLiteral(const string& str) {
    if (str.length() < 2 || str.front() != '"' || str.back() != '"') {
        return false;
    }
    
    // Count trailing backslashes right before the last quote
    int backslashCount = 0;
    for (int i = (int)str.length() - 2; i >= 0; i--) {
        if (str[i] == '\\') backslashCount++;
        else break;
    }

    // If backslash count is EVEN, the quote is REAL. If ODD, it is ESCAPED (\").
    return (backslashCount % 2 == 0);
}
// Function to read space-separated tokens from output1.txt and write tokens to output2.txt
void processTokenization() {
    if (!initializeFiles(step1FileName, step2FileName)) return;

    char buffer[512];
    while (fscanf(inFile, "%s", buffer) != EOF) {
        string token = buffer;

        // If it starts with '"', keep reading until a REAL closing quote is found
        if (token.front() == '"' && !isCompleteStringLiteral(token)) {
            char nextWord[512];
            while (fscanf(inFile, "%s", nextWord) != EOF) {
                token += " ";
                token += nextWord;
                if (isCompleteStringLiteral(token)) break;
            }
        }

        string tokenTag = categorizeToken(token);
        fprintf(outFile, "%s ", tokenTag.c_str());
    }

    closeFiles();
}

// ============================================================================
// CONSOLE DISPLAY MODULE
// ============================================================================
void displayFileContent(const char* filename, const char* label) {
    FILE* file = fopen(filename, "r");
    
    cout << "\n=====================================================\n";
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
    cout << "\n=====================================================\n";
    fclose(file);
}