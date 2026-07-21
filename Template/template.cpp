#include <bits/stdc++.h>
using namespace std;

// ============================================================================
// GLOBAL CONFIGURATION & FILE MANAGEMENT
// ============================================================================
FILE* inFile  = NULL;
FILE* outFile = NULL;

const char* INPUT_FILE    = "input.cpp";
const char* FILTERED_FILE = "step1_filtered.txt";
const char* LEXEME_FILE   = "step2_lexemes.txt";
const char* TOKEN_FILE    = "step3_tokens.txt";

// Token Classification Sets
unordered_set<string> keywords = {
    "#include", "char", "int", "float", "double", "if", "else", "while", "for", 
    "return", "void", "string", "cout", "cin", "using", "namespace"
};
unordered_set<string> operators = {
    "=", "+", "-", "*", "/", "%", "==", "!=", "<", ">", "<=", ">=", 
    "+=", "-=", "*=", "/=", "++", "--", "<<", ">>"
};
unordered_set<string> parentheses = { "(", ")", "{", "}", "[", "]" };
unordered_set<string> separators  = { ";", ",", "\'", "\"" };

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================
bool openFiles(const char* inPath, const char* outPath);
void closeFiles();
void displayFileContent(const char* filename, const char* label);

// Phase 1: Filtering (Assignment 1)
void phase1_filterSourceCode();
void p1_handleMultiLineComment(char ch, bool& inMulti, bool& lastSpace);
void p1_handleSingleLineComment(char ch, bool& inSingle, bool& lastSpace);
void p1_handleStringLiteral(char ch, bool& inStr, bool& lastSpace);

// Phase 2: Lexeme Separation (Assignment 2 - Step 1)
void phase2_separateLexemes();
bool isSymbol(char ch);
bool isCompoundOperator(char ch, char nextCh);

// Phase 3: Tokenization & DFAs (Assignment 2 - Step 2)
void phase3_tokenizeLexemes();
bool isIdentifierDFA(const string& token);
bool isNumberDFA(const string& token);
bool isCompleteStringLiteral(const string& str);
string categorizeToken(const string& token);

// ============================================================================
// MAIN PIPELINE CONTROLLER
// ============================================================================
int main() {
    cout << "Starting Compiler Lexical Analysis Pipeline...\n";

    // ---------------------------------------------------------
    // PHASE 1: Remove Comments & Compress Whitespace
    // ---------------------------------------------------------
    if (!openFiles(INPUT_FILE, FILTERED_FILE)) return 1;
    phase1_filterSourceCode();
    closeFiles();

    // ---------------------------------------------------------
    // PHASE 2: Isolate Lexemes with Spaces
    // ---------------------------------------------------------
    if (!openFiles(FILTERED_FILE, LEXEME_FILE)) return 1;
    phase2_separateLexemes();
    closeFiles();

    // ---------------------------------------------------------
    // PHASE 3: Tokenize using DFAs and Lookups
    // ---------------------------------------------------------
    if (!openFiles(LEXEME_FILE, TOKEN_FILE)) return 1;
    phase3_tokenizeLexemes();
    closeFiles();

    // ---------------------------------------------------------
    // DISPLAY RESULTS
    // ---------------------------------------------------------
    displayFileContent(INPUT_FILE, "ORIGINAL INPUT SOURCE CODE");
    displayFileContent(FILTERED_FILE, "PHASE 1: FILTERED CODE (No Comments/Extra Space)");
    displayFileContent(LEXEME_FILE, "PHASE 2: SEPARATED LEXEMES");
    displayFileContent(TOKEN_FILE, "PHASE 3: TOKENIZED CATEGORIES (Final Output)");

    return 0;
}

// ============================================================================
// FILE MANAGEMENT HELPERS
// ============================================================================
bool openFiles(const char* inPath, const char* outPath) {
    inFile  = fopen(inPath, "r");
    outFile = fopen(outPath, "w");
    if (!inFile || !outFile) {
        cerr << "Error opening files: " << inPath << " or " << outPath << endl;
        return false;
    }
    return true;
}

void closeFiles() {
    if (inFile) fclose(inFile);
    if (outFile) fclose(outFile);
}

void displayFileContent(const char* filename, const char* label) {
    FILE* file = fopen(filename, "r");
    if (!file) return;
    
    cout << "\n====================================================================\n";
    cout << "  " << label << " (" << filename << ")\n";
    cout << "====================================================================\n";
    
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        cout << buffer;
    }
    cout << "\n====================================================================\n";
    fclose(file);
}

// ============================================================================
// PHASE 1: COMMENT REMOVAL & WHITESPACE FILTERING
// ============================================================================
void phase1_filterSourceCode() {
    bool inMultiLine = false, inSingleLine = false;
    bool inString = false, lastCharWasSpace = false;
    int c;

    while ((c = fgetc(inFile)) != EOF) {
        char ch = (char)c;

        if (inMultiLine) {
            p1_handleMultiLineComment(ch, inMultiLine, lastCharWasSpace);
        } 
        else if (inSingleLine) {
            p1_handleSingleLineComment(ch, inSingleLine, lastCharWasSpace);
        } 
        else if (inString) {
            p1_handleStringLiteral(ch, inString, lastCharWasSpace);
        } 
        else {
            if (ch == '"') {
                inString = true;
                fputc(ch, outFile);
                lastCharWasSpace = false;
                continue;
            }
            if (ch == '/') {
                char nextCh = fgetc(inFile);
                if (nextCh == '/') { inSingleLine = true; continue; }
                else if (nextCh == '*') { inMultiLine = true; continue; }
                else { ungetc(nextCh, inFile); }
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
}

void p1_handleMultiLineComment(char ch, bool& inMulti, bool& lastSpace) {
    if (ch == '*') {
        char nextCh = fgetc(inFile);
        if (nextCh == '/') {
            inMulti = false;
            // Pad with space to prevent token merging (e.g., int/*cmt*/y -> int y)
            if (!lastSpace) {
                fputc(' ', outFile);
                lastSpace = true;
            }
        } else {
            ungetc(nextCh, inFile);
        }
    }
}

void p1_handleSingleLineComment(char ch, bool& inSingle, bool& lastSpace) {
    if (ch == '\n') {
        inSingle = false;
        lastSpace = false; 
    }
}

void p1_handleStringLiteral(char ch, bool& inStr, bool& lastSpace) {
    if (ch == '\\') { // Handle escaped quotes inside strings
        fputc(ch, outFile);
        int nextC = fgetc(inFile);
        if (nextC != EOF) fputc((char)nextC, outFile);
        return;
    }
    if (ch == '"') inStr = false;
    fputc(ch, outFile);
    lastSpace = false;
}

// ============================================================================
// PHASE 2: LEXEME SEPARATION
// ============================================================================
bool isSymbol(char ch) {
    // Note: '.' is omitted to prevent breaking floating point numbers like 100.0
    string symbols = "=+-*/%<>&|^!;,(){}[]";
    return symbols.find(ch) != string::npos;
}

bool isCompoundOperator(char ch, char nextCh) {
    return (ch == '=' && nextCh == '=') || (ch == '<' && nextCh == '=') ||
           (ch == '>' && nextCh == '=') || (ch == '!' && nextCh == '=') ||
           (ch == '+' && nextCh == '=') || (ch == '-' && nextCh == '=') ||
           (ch == '*' && nextCh == '=') || (ch == '/' && nextCh == '=') ||
           (ch == '<' && nextCh == '<') || (ch == '>' && nextCh == '>');
}

void phase2_separateLexemes() {
    int c;
    bool inString = false;
    bool isEscaped = false;
    bool lastCharWasSpace = false;

    // Helper lambda for safe spacing
    auto writeSpace = [&]() {
        if (!lastCharWasSpace) { fputc(' ', outFile); lastCharWasSpace = true; }
    };
    auto writeChar = [&](char ch) {
        fputc(ch, outFile); lastCharWasSpace = (ch == ' ');
    };

    while ((c = fgetc(inFile)) != EOF) {
        char ch = (char)c;

        if (inString) {
            writeChar(ch);
            if (isEscaped) isEscaped = false;
            else if (ch == '\\') isEscaped = true;
            else if (ch == '"') inString = false;
            continue;
        }

        if (ch == '"') {
            inString = true;
            isEscaped = false;
            writeChar(ch);
            continue;
        }

        if (isSymbol(ch)) {
            int nextC = fgetc(inFile);
            char nextCh = (nextC != EOF) ? (char)nextC : '\0';

            if (nextC != EOF && isCompoundOperator(ch, nextCh)) {
                writeSpace();
                fputc(ch, outFile);
                fputc(nextCh, outFile);
                fputc(' ', outFile);
                lastCharWasSpace = true;
            } else {
                if (nextC != EOF) ungetc(nextC, inFile);
                writeSpace();
                fputc(ch, outFile);
                fputc(' ', outFile);
                lastCharWasSpace = true;
            }
        } else {
            if (isspace(ch)) writeSpace();
            else writeChar(ch);
        }
    }
}

// ============================================================================
// PHASE 3: TOKENIZATION AND DFA ENGINE
// ============================================================================

/**
 * Mandatory DFA for Identifiers
 * Accepts: Alphanumeric strings starting with letter or underscore
 */
bool isIdentifierDFA(const string& token) {
    int state = 0;
    for (char ch : token) {
        if (state == 0) {
            if (isalpha(ch) || ch == '_') state = 1;
            else state = 2; // Trap state
        } else if (state == 1) {
            if (isalnum(ch) || ch == '_') state = 1;
            else state = 2;
        } else if (state == 2) {
            state = 2;
        }
    }
    return (state == 1);
}

/**
 * Mandatory DFA for Numeric Constants
 * Accepts: Integers (e.g., 5) and Floats (e.g., 2.5). Traps malformed (e.g., 100.05.5)
 */
bool isNumberDFA(const string& token) {
    int state = 0;
    for (char ch : token) {
        if (state == 0) {
            if (isdigit(ch)) state = 1;
            else if (ch == '.') state = 2;
            else state = 4; // Trap state
        } else if (state == 1) {
            if (isdigit(ch)) state = 1;
            else if (ch == '.') state = 2;
            else state = 4;
        } else if (state == 2) {
            if (isdigit(ch)) state = 3;
            else state = 4;
        } else if (state == 3) {
            if (isdigit(ch)) state = 3;
            else state = 4;
        } else if (state == 4) {
            state = 4;
        }
    }
    return (state == 1 || state == 3);
}

// Verifies if string ends with a genuine unescaped quote
bool isCompleteStringLiteral(const string& str) {
    if (str.length() < 2 || str.front() != '"' || str.back() != '"') return false;
    int backslashCount = 0;
    for (int i = (int)str.length() - 2; i >= 0; i--) {
        if (str[i] == '\\') backslashCount++;
        else break;
    }
    return (backslashCount % 2 == 0); // Even count = real quote, Odd count = escaped quote
}

string categorizeToken(const string& token) {
    if (token.length() >= 2 && token.front() == '"' && token.back() == '"') return "[str " + token + "]";
    if (keywords.find(token) != keywords.end()) return "[kw " + token + "]";
    if (operators.find(token) != operators.end()) return "[op " + token + "]";
    if (parentheses.find(token) != parentheses.end()) return "[par " + token + "]";
    if (separators.find(token) != separators.end()) return "[sep " + token + "]";
    if (isIdentifierDFA(token)) return "[id " + token + "]";
    if (isNumberDFA(token)) return "[num " + token + "]";
    return "[unkn " + token + "]";
}

void phase3_tokenizeLexemes() {
    char buffer[512];
    while (fscanf(inFile, "%s", buffer) != EOF) {
        string token = buffer;

        // Reassemble strings that were split by internal spaces during space-separated reads
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
}