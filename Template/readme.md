# Compiler Design: Complete Lexical Analysis Pipeline

**Author:** [Your Name / Student ID]  
**Course:** Formal Language and Compilers Lab (CSE 4130)  
**Institution:** Ahsanullah University of Science and Technology (AUST)

---

## 1. Problem Description & Breakdown

The primary step in compiling a C/C++ program is **Lexical Analysis**, which transforms a raw stream of characters into a structured sequence of meaningful tokens. Source code written by humans contains noise (like comments and arbitrary whitespace) and attached syntactical units (like `x+=5;`) that make direct parsing impossible.

This project tackles the lexical analysis phase by breaking it down into three distinct objectives:

1. **Noise Filtering (Phase 1):** Remove all single-line (`//`) and multi-line (`/* ... */`) comments, and compress redundant spaces, tabs, and newlines into single spaces[cite: 1].
2. **Lexeme Separation (Phase 2):** Scan the cleaned code to isolate operators, separators, and brackets by padding them with spaces, ensuring compound operators (e.g., `==`, `<=`) and string literals are kept intact[cite: 5].
3. **Token Classification (Phase 3):** Read the isolated lexemes word-by-word and classify them into predefined token categories (Keywords, Identifiers, Numbers, Operators, Separators, Strings, or Unknown) using explicit Deterministic Finite Automata (DFAs)[cite: 5].

---

## 2. Proposed Solution & Function Overview

The solution implements a sequential, three-phase pipeline. The output of each phase is written to a temporary file, which serves as the input for the next phase. This ensures strict modularity and state isolation.

### A. Core File Management

- `openFiles(const char* inPath, const char* outPath)`: Safely opens the input stream for reading and output stream for writing.
- `closeFiles()`: Closes active file pointers to prevent memory leaks and ensure data is flushed to disk.

### B. Phase 1: Noise Filtering Engine

- `phase1_filterSourceCode()`: The main loop for Phase 1. Reads characters sequentially and routes them to specific handlers based on active state flags (e.g., inside a comment or string).
- `p1_handleMultiLineComment()` / `p1_handleSingleLineComment()`: Skips characters until the respective comment closure (`*/` or `\n`) is found. Inserts necessary padding spaces to prevent token merging (e.g., `int/*cmt*/x` becomes `int x`).
- `p1_handleStringLiteral()`: Safely preserves strings, effectively handling escape sequences (`\"` or `\\`) to prevent premature termination.

### C. Phase 2: Lexeme Separation Engine

- `phase2_separateLexemes()`: Iterates through the filtered code, identifying attached symbols and padding them with spaces.
- `isSymbol(char ch)`: Returns true for valid single-character operators and separators (ignoring `.` to protect floating-point numbers).
- `isCompoundOperator(char ch, char nextCh)`: Look-ahead function that validates 2-character relational and arithmetic operators[cite: 5].

### D. Phase 3: Tokenization Engine & DFAs

- `phase3_tokenizeLexemes()`: Reads space-separated lexemes from Phase 2. Automatically reassembles string literals containing spaces, and categorizes tokens.
- `isIdentifierDFA(const string& token)`: A state machine that strictly accepts strings starting with a letter/underscore and followed by alphanumeric characters[cite: 5].
- `isNumberDFA(const string& token)`: A state machine that verifies integer and floating-point validity, trapping malformed inputs (e.g., `100.05.5`).
- `categorizeToken(const string& token)`: Evaluates the token against Hash Sets (for keywords, operators, separators) and the DFAs, returning the formatted tag (e.g., `[kw int]`, `[id x]`, `[num 5]`).

---

## 3. System Processing Flowchart

```text
+-------------------------+
|   Raw Input C++ Code    |
|      (input.cpp)        |
+-----------+-------------+
            |
            v
+-------------------------+      +------------------------------------+
|  Phase 1: Filter Engine | ---> | p1_handleMultiLineComment()        |
| (Removes comments/gaps) |      | p1_handleSingleLineComment()       |
+-----------+-------------+      +------------------------------------+
            |
            v
+-------------------------+      +------------------------------------+
|   step1_filtered.txt    | ---> | "int x=5; /*cmt*/ x+=10;"          |
+-----------+-------------+      | becomes: "int x=5; x+=10;"         |
            |                    +------------------------------------+
            v
+-------------------------+      +------------------------------------+
| Phase 2: Lexeme Engine  | ---> | isSymbol(), isCompoundOperator()   |
| (Pads symbols w/ space) |      | Detects strings & preserves them   |
+-----------+-------------+      +------------------------------------+
            |
            v
+-------------------------+      +------------------------------------+
|   step2_lexemes.txt     | ---> | "int x=5;"                         |
+-----------+-------------+      | becomes: "int x = 5 ;"             |
            |                    +------------------------------------+
            v
+-------------------------+      +------------------------------------+
| Phase 3: Token Engine   | ---> | isIdentifierDFA(), isNumberDFA()   |
| (Applies DFAs & lookups)|      | categorizeToken()                  |
+-----------+-------------+      +------------------------------------+
            |
            v
+-------------------------+      +------------------------------------+
|   step3_tokens.txt      | ---> | [kw int] [id x] [op =] [num 5] ... |
+-------------------------+      +------------------------------------+

```

---

## 4. Conclusion

This project successfully bridges the gap between raw, human-written source code and structured lexical tokens. By dividing the process into filtering, separation, and DFA-based tokenization, the pipeline robustly handles edge cases like escaped string quotes, compound operators, and malformed decimals. This implementation serves as a fully functional and strictly standard Lexical Analyzer frontend, ready to feed a syntactic parser.

## 5. Authorship

- **Author:** [Your Name]
- **Student ID:** [Your ID]
- **Semester:** Fall 2025

- **Language / Compiler:** C++17 (GCC)

---
