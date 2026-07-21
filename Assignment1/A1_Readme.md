# Compiler Design Assignment 1: Comment Removal and Whitespace Filtering

**Author:** Sadia Sultana  
**Course:** CSE 4130 - Formal Language and Compilers Lab  
**Institution:** Ahsanullah University of Science and Technology (AUST)

---

## 1. Problem Description & Breakdown

In compiler design, the lexical analysis pipeline begins with pre-processing source code to eliminate non-syntactic elements such as comments and redundant whitespace. This process reduces the character stream size and ensures that the downstream lexer processes only active, syntactically meaningful code.

### Objectives

- **Remove Single-Line Comments (`//`):** Eliminate all characters from `//` up to the end-of-line (`\n`).
- **Remove Multi-Line Comments (`/* ... */`):** Eliminate all characters enclosed between `/*` and `*/`.
- **Compress Redundant Whitespace:** Replace consecutive spaces, tabs (`\t`), and newlines (`\n`) with a single space.
- **Prevent Token Merging:** Ensure that removing inline comments between two identifiers or keywords (e.g., `int/*comment*/y`) inserts a separating space to preserve distinct syntax tokens (yielding `int y`).
- **Isolate String Literals:** Ensure string literals (e.g., `"/* fake comment */"`, `"// comment"`, or escaped strings like `"\""`) remain completely unaltered during pre-processing.

---

## 2. Proposed Architectural Solution Overview

The program implements a state-driven character stream processor using standard C file pointers (`FILE*`) and global boolean flags to maintain processing states across function calls.

```text
[ input.cpp ] ──► (runCoreProcessingEngine) ──► [ state handlers ] ──► [ output.cpp ]

```

1. **Initialization Phase:** Opens source code (`input.cpp`) for reading and destination file (`output.cpp`) for writing using modular file managers (`initializeFiles`).
2. **State Machine Execution Phase:** Reads input character-by-character via `fgetc()`. Based on state flags, delegates processing to dedicated state handlers:

- **Multi-Line Comment Mode:** Consumes characters until finding `*/`.
- **Single-Line Comment Mode:** Consumes characters until finding `\n`.
- **String Literal Mode:** Copies characters verbatim, handling backslash escapes (`\`) to prevent premature termination on escaped quotes (`\"`).
- **Baseline Active Code Mode:** Detects comment signatures (`//`, `/*`), string starts (`"`), or compresses active whitespace.

3. **Console Verification Phase:** Outputs the full contents of both `input.cpp` and `output.cpp` to stdout using `displayFileContent()`.

---

## 3. Detailed Breakdown of Solution & Modular Functions

The code is structured modularly with clear separation of file management, state machine execution, state-specific handlers, and console display logic.

### A. File Management Module

- `bool initializeFiles(const char* inputSource, const char* outputSource)`
- **Usage:** Opens `inputSource` in read mode (`"r"`) and `outputSource` in write mode (`"w"`).
- **Handled Case:** Safely closes any successfully opened handle and returns `false` if either file pointer evaluates to `NULL`.

### B. Core State Machine Engine

- `void runCoreProcessingEngine()`
- **Usage:** The primary character-processing loop. Reads `inFile` using `fgetc()` until `EOF`.
- **Handled Case:** Checks active flags (`inMultiLineComment`, `inSingleLineComment`, `inString`) and routes execution to state handlers. If no flag is raised, inspects active code for string quotes (`"`), slash operators (`/`), look-ahead checks (`fgetc`/`ungetc`), and whitespace compression using `lastCharWasSpace`.

### C. Specialized State Handlers

- `void handleMultiLineComment(char ch)`
- **Usage:** Invoked when `inMultiLineComment` is `true`.
- **Handled Case:** Ignores comment characters. Upon detecting `*`, peeks the next character via `fgetc()`. If `/` is found, resets `inMultiLineComment = false` and writes a single space (`' '`) to prevent merging surrounding tokens (e.g., `int/*comment*/y` $\rightarrow$ `int y`).

- `void handleSingleLineComment(char ch)`
- **Usage:** Invoked when `inSingleLineComment` is `true`.
- **Handled Case:** Discards characters until `ch == '\n'`, whereupon it resets `inSingleLineComment = false`.

- `void handleStringLiteral(char ch)`
- **Usage:** Invoked when `inString` is `true`.
- **Handled Case:** If an escape character (`\`) is encountered, it writes the backslash and immediately fetches and writes the subsequent character, bypassing normal string quote termination checks. Unescaped quotes (`"`) reset `inString = false`.

### D. Console Output Module

- `void displayFileContent(const char* filename, const char* label)`
- **Usage:** Reopens a target file in read mode and prints its contents line-by-line to the terminal using `fgets()` with custom header/footer formatting.

---

## 4. Line-by-Line Execution Flow Trace

Let's trace two representative code blocks from the input:

### Trace 1: Token Merging Prevention (`int/*comment*/y = 50;`)

1. `main()` calls `runCoreProcessingEngine()`.
2. Loop processes `i`, `n`, `t`. Writes `int` to `outFile`.
3. Loop encounters `/`. Peeks next character: finds `*`.

- Sets `inMultiLineComment = true`.

4. Subsequent calls read `c`, `o`, `m`, `m`, `e`, `n`, `t` inside `handleMultiLineComment()` (all discarded).
5. Loop reads `*`. Peeks next character: finds `/`.

- Sets `inMultiLineComment = false`.
- Checks `!lastCharWasSpace` $\rightarrow$ `true`. Writes `' '` to `outFile` and sets `lastCharWasSpace = true`.

6. Loop reads `y`. Writes `y` to `outFile`.

- **Result in Output:** `int y = 50;` (Tokens stay distinct).

---

### Trace 2: String Literal Isolation (`string stringA = "fake /* comment */";`)

1. `runCoreProcessingEngine()` reads active code up to `"`.

- Sets `inString = true` and writes `"` to `outFile`.

2. Subsequent characters are routed to `handleStringLiteral()`.
3. Loop reads `/`, `*`, `f`, `a`, `k`, `e`, ` `, `/*`, ` `, `c`, `o`, `m`, `m`, `e`, `n`, `t`, ` `, `*/`.

- Because `inString` is `true`, `handleStringLiteral()` writes every character directly to `outFile` without triggering comment logic.

4. Unescaped closing `"` resets `inString = false`.

- **Result in Output:** `"fake /* comment */"` (Literal preserved).

---

## 5. System Processing Flowchart

```text
                     +-------------------+
                     |      main()       |
                     +---------+---------+
                               |
                               v
                     +-------------------+
                     | initializeFiles() |
                     +---------+---------+
                               |
                               v
                     +-------------------+
                     | runCoreProcess-   |
                     |   ingEngine()     |
                     +---------+---------+
                               |
                       (Loop: fgetc)
                               |
        +----------------------+----------------------+
        |                      |                      |
        v (inMultiLine)        v (inSingleLine)       v (inString)
+-------------------+  +-------------------+  +-------------------+
| handleMultiLine   |  | handleSingleLine  |  | handleString      |
|    Comment()      |  |    Comment()      |  |     Literal()     |
+-------------------+  +-------------------+  +-------------------+
        |                      |                      |
        +----------------------+----------------------+
                               |
                               v (Active Code)
            +------------------+------------------+
            |                  |                  |
            v                  v                  v
     (Is String Quote?)  (Is Slash / ?)    (Is Whitespace?)
            |                  |                  |
            v                  v                  v
     Set inString=true  Check next character  Compress multiple
     Write Quote        '/' -> SingleLine     spaces into ' '
                        '*' -> MultiLine
                        else -> ungetc
                               |
                               v
                     +-------------------+
                     |displayFileContent |
                     +-------------------+

```

---

## 6. Verification & Output Comparison

### Input Source (`input.cpp`):

```cpp
#include <bits/stdc++.h>
using namespace std;

int main() {
    // 1. Basic Operations & Collapsible Whitespace Traps
    int       x      =      5     ;
    x += 10;

    // 2. Standard Multi-Line Comment
    /* This is a simple
       multi-line comment block */

    // 3. String Literal Isolation Verification
    string stringA = "String containing a literal double slash // and a /* fake comment */ block";
    string stringB = "String containing escaped tokens: \" \\\" \\t \\n and an unescaped space   ";

    // 4. Token Merging Prevention Verification
    // Removing comments must not merge separate syntax tokens.
    int/*comment*/y = 50;
    float /*comment*/ z = 100.0;

    // 5. Comment Signatures within String Lookalikes
    cout << "Check path: C:\\Users\\Name\\Documents\\file.txt" << endl;

    // 6. Slashes used as arithmetic division operators (Not comments!)
    int result = x / y;
    int fractional = y / /* inline noise */ z;
    cout << "End of the input file." << endl;
    return 0;
}

```

### Filtered Output Source (`output.cpp`):

```cpp
#include <bits/stdc++.h> using namespace std; int main() {  int x = 5 ; x += 10;   string stringA = "String containing a literal double slash // and a /* fake comment */ block"; string stringB = "String containing escaped tokens: \" \\\" \\t \\n and an unescaped space   ";   int y = 50; float z = 100.0;  cout << "Check path: C:\\Users\\Name\\Documents\\file.txt" << endl;  int result = x / y; int fractional = y / z; cout << "End of the input file." << endl; return 0; }

```

---

## 7. Conclusion & Authorship

### Conclusion

The state-driven character filtering engine successfully eliminates single-line and multi-line comments while compressing redundant whitespaces. By isolating string literals and inserting padding spaces during multi-line comment terminations, the pre-processor preserves syntactic structure and token boundaries required for Assignment 2.

### Authorship

- **Implementation & Design:** Sadia Sultana
- **Repository:** cse_4130_FormalLanguages-CompilersLab
- **Language/Standard:** C++17 (GCC / Standard C IO library)
