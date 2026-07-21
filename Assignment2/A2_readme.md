# Compiler Design Assignment 2: Lexeme Separation and Tokenization using DFAs

**Author:** Sadia Sultana | 20220204035  
**Course:** Formal Languages and Compilers Design Lab

---

## 1. Problem Description & Breakdown

In compiler design, the lexical analysis phase (Lexer) is responsible for converting a raw stream of characters from a source program into a sequence of meaningful atomic units called **lexemes**, and assigning them corresponding syntactic categories called **tokens**.

This assignment takes the cleaned output of Assignment 1 (which removed comments and excessive whitespaces) and processes it through a two-step lexical pipeline:

### Step 1: Lexeme Separation

- Raw source code often contains operators, separators, and brackets attached to identifiers or literals without intervening spaces (e.g., `x=5;`, `main()`, `100.o5;`).
- **Goal:** Process the input character-by-character and insert space paddings around all isolated operators and separators while preserving compound 2-character operators (like `+=`, `==`, `<=`) and string literals (including escaped quotes like `\"`) intact.

### Step 2: Token Classification using DFAs

- Takes the space-separated lexemes produced by Step 1 and categorizes each unit into its specific token class.
- **Goal:** Use explicit Deterministic Finite Automata (DFAs) for **Identifiers** (`id`) and **Numeric Constants** (`num`), while utilizing fast lookup sets for **Keywords** (`kw`), **Operators** (`op`), **Parentheses** (`par`), **Separators** (`sep`), and **String Literals** (`str`). Any token failing all valid rules is categorized as **Unknown** (`unkn`).

---

## 2. Proposed Architectural Solution Overview

The processing pipeline follows a single-pass, modular flow executed sequentially from the `main()` controller:

```text
[ output.cpp ] ──► (Step 1: processLexemeSeparation) ──► [ output1.txt ] ──► (Step 2: processTokenization) ──► [ output2.txt ]

```

1. **Initialization Phase:** Global file management handles opening and closing file descriptors (`FILE*`) safely using custom modular file functions (`initializeFiles`, `closeFiles`).
2. **Lexeme Separation Phase:** Processes character stream via look-ahead character inspection using `fgetc` and `ungetc`, preventing duplicate spaces via `lastCharWasSpace` and maintaining escaped string states via an `isEscaped` boolean tracker.
3. **Tokenization Phase:** Iterates through isolated lexemes word-by-word using `fscanf`, reassembling string literals that contain spaces/escaped quotes via `isCompleteStringLiteral`, and running each token through a priority classifier sequence (`String Literal` $\rightarrow$ `Keywords` $\rightarrow$ `Operators` $\rightarrow$ `Parentheses` $\rightarrow$ `Separators` $\rightarrow$ `Identifier DFA` $\rightarrow$ `Numeric DFA` $\rightarrow$ `Unknown`).
4. **Console Display Phase:** Renders input and intermediate results for immediate visual verification.

---

## 3. Detailed Breakdown of Solution & Modular Functions

The code is strictly modularized with single-responsibility functions that interact via global file state variables.

### A. File Management Module

- `bool initializeFiles(const char* inPath, const char* outPath)`
- **Usage:** Opens input (`"r"`) and output (`"w"`) files assigned to global pointers `inFile` and `outFile`.
- **Handled Case:** Displays error message and returns `false` if file path is missing or inaccessible.

- `void closeFiles()`
- **Usage:** Safely closes non-null global file streams.

### B. Character Inspection & Output Module (Step 1)

- `bool isSymbol(char ch)`
- **Usage:** Identifies single-character symbols (`=`, `+`, `-`, `*`, `/`, `%`, `<`, `>`, `;`, `,`, `(`, `)`, `{`, `}`, etc.).

- `bool isCompoundOperator(char ch, char nextCh)`
- **Usage:** Checks if two adjacent characters form a 2-character compound operator (`==`, `<=`, `>=`, `!=`, `+=`, `-=`, `*=`, `/=`, `<<`, `>>`).

- `void writeSpace()`
- **Usage:** Writes a `' '` to `outFile` **only** if `lastCharWasSpace` is `false`. Prevents duplicate spaces dynamically.

- `void writeChar(char ch)`
- **Usage:** Writes character `ch` to `outFile` and updates `lastCharWasSpace = (ch == ' ')`.

### C. Character Handling Module (Step 1 Engine)

- `void handleSymbol(char ch)`
- **Usage:** Reads look-ahead character via `fgetc`. If `isCompoundOperator` is true, writes both characters padded by spaces. Otherwise, uses `ungetc` to push back the peeked character and pads the single symbol with spaces.

- `void handleRegularCode(char ch)`
- **Usage:** Converts whitespace into standard spaces via `writeSpace()` or writes alphanumeric characters using `writeChar(ch)`.

- `void processLexemeSeparation()`
- **Usage:** Top-level loop reading character-by-character. Tracks state flag `inString` and `isEscaped` so characters inside string literals (e.g. `"escaped \" quote"`) are preserved without padding.

### D. DFA & Classification Module (Step 2 Engine)

- `bool isIdentifierDFA(const string& token)`
- **State Machine:**
- **State 0 (Start):** Letter/Underscore $\rightarrow$ State 1; Digit/Symbol $\rightarrow$ State 2 (Trap).
- **State 1 (Accepting):** Alphanumeric/Underscore $\rightarrow$ State 1 (Self-loop); Symbol $\rightarrow$ State 2 (Trap).
- **State 2 (Trap State):** Self-loop.

- **Returns:** `true` if loop terminates in State 1.

- `bool isNumberDFA(const string& token)`
- **State Machine:**
- **State 0 (Start):** Digit $\rightarrow$ State 1; Dot (`.`) $\rightarrow$ State 2; Other $\rightarrow$ State 4 (Trap).
- **State 1 (Accepting Int):** Digit $\rightarrow$ State 1; Dot (`.`) $\rightarrow$ State 2; Other $\rightarrow$ State 4.
- **State 2 (Expecting Fraction):** Digit $\rightarrow$ State 3; Other (e.g. letter `'o'` in `'100.o5'`) $\rightarrow$ State 4 (Trap).
- **State 3 (Accepting Float):** Digit $\rightarrow$ State 3; Other $\rightarrow$ State 4.
- **State 4 (Trap State):** Self-loop.

- **Returns:** `true` if loop terminates in State 1 or State 3.

- `bool isCompleteStringLiteral(const string& str)`
- **Usage:** Verifies whether a string starting with `"` ends with a real closing quote by counting preceding backslashes (even count = real quote, odd count = escaped quote `\"`).

- `string categorizeToken(const string& token)`
- **Usage:** Evaluates token priority against hash sets and DFAs. Returns formatted token tags (e.g., `"[str \"...\"]"`, `"[kw int]"`, `"[id x]"`, `"[unkn .h]"`).

- `void processTokenization()`
- **Usage:** Opens Step 1 & Step 2 files using `initializeFiles`, loops using `fscanf(inFile, "%s", buffer)`, reassembles strings, categorizes tokens, writes tags to `outFile`, and calls `closeFiles()`.

---

## 4. Line-by-Line Execution Flow Trace

Let's trace the execution of a snippet from the input source:

```cpp
int x = 5 ; string stringB = "String containing escaped tokens: \" \\\" " ;

```

### Trace 1: Main $\rightarrow$ Step 1 (Lexeme Separation)

1. `main()` calls `initializeFiles("output.cpp", "output1.txt")`. Pointers `inFile` and `outFile` are initialized.
2. `main()` calls `processLexemeSeparation()`.
3. Loop reads character `'i'`:

- `isSymbol('i')` is `false` $\rightarrow$ calls `handleRegularCode('i')` $\rightarrow$ calls `writeChar('i')`.

4. Loop reads `'n'`, `'t'`, `' '`: Reads whitespace, calls `writeSpace()`.
5. Loop encounters `x`, `' '`, `=`:

- Reads `'='` $\rightarrow$ `isSymbol('=')` is `true` $\rightarrow$ calls `handleSymbol('=')`.
- `handleSymbol` peeks next character using `fgetc` $\rightarrow$ sees `' '`.
- `isCompoundOperator('=', ' ')` is `false`.
- Calls `ungetc(' ', inFile)` to return `' '` to stream.
- Calls `writeSpace()`, outputs `'='`, outputs `' '`, sets `lastCharWasSpace = true`.

6. Loop encounters `" String containing... \" ... "`:

- Sees `"` $\rightarrow$ sets `inString = true`.
- Encounters `\"` $\rightarrow$ sees `\`, sets `isEscaped = true`.
- Next character `"` is preserved inside the string because `isEscaped` is active.
- Final unescaped `"` sets `inString = false`.

7. `processLexemeSeparation()` terminates at `EOF`.
8. `main()` calls `closeFiles()`.

- **Output in `output1.txt`:** `int x = 5 ; string stringB = "String containing escaped tokens: \" \\\" " ;`

---

### Trace 2: Main $\rightarrow$ Step 2 (Tokenization)

1. `main()` calls `processTokenization()`.
2. `processTokenization()` calls `initializeFiles("output1.txt", "output2.txt")`.
3. `fscanf(inFile, "%s", buffer)` extracts first token: **`"int"`**.

- Calls `categorizeToken("int")`.
- `keywords.find("int")` matches $\rightarrow$ returns `"[kw int]"`.
- `fprintf(outFile, "%s ", "[kw int]")`.

4. Next token: **`"x"`**.

- Keywords/Operators/Separators fail.
- Calls `isIdentifierDFA("x")`:
- State 0 $\rightarrow$ sees `'x'` (alpha) $\rightarrow$ State 1.
- End of string reached in State 1 $\rightarrow$ returns `true`.

- `categorizeToken` returns `"[id x]"`.

5. Next token: **`"="`** $\rightarrow$ Matches `operators` $\rightarrow$ returns `"[op =]"`.
6. Next token: **`"5"`** $\rightarrow$ `isNumberDFA("5")` ends in State 1 $\rightarrow$ returns `"[num 5]"`.
7. Next token: **`"\"String`**:

- Token starts with `"`. `isCompleteStringLiteral("\"String")` returns `false`.
- `processTokenization()` loops with `fscanf`, appending words until `isCompleteStringLiteral` finds the unescaped closing `"`.
- `categorizeToken` evaluates complete string $\rightarrow$ returns `"[str \"String containing escaped tokens: \" \\\"]"`.

8. `processTokenization()` completes loop at `EOF` and calls `closeFiles()`.
9. Control returns to `main()`, displays results, and exits with return code `0`.

---

## 5. System Processing Flowchart

```text
                     +-------------------+
                     |      main()       |
                     +---------+---------+
                               |
            +------------------+------------------+
            |                                     |
            v                                     v
+-----------------------+             +-----------------------+
|  initializeFiles()    |             |  processTokenization()|
|  (output.cpp,         |             |  (output1.txt,        |
|   output1.txt)        |             |   output2.txt)        |
+-----------+-----------+             +-----------+-----------+
            |                                     |
            v                                     | (Calls initializeFiles)
+-----------------------+                         v
|processLexemeSepar-    |             +-----------------------+
|       ation()         |             |   fscanf token loop   |
+-----------+-----------+             +-----------+-----------+
            |                                     |
    +-------+-------+                             v
    |               |                 +-----------------------+
    v               v                 |isCompleteStringLiteral|
+-------+       +-------+             +-----------+-----------+
|isSym- |       |handle-|                         |
| bol() |       |Symbol |                         v
+-------+       +-------+             +-----------------------+
                                      |  categorizeToken()    |
                                      +-----------+-----------+
                                                  |
        +-----------------+-----------------+-----+-----------+-----------------+
        |                 |                 |                 |                 |
        v                 v                 v                 v                 v
  +-----------+     +-----------+     +-----------+     +-----------+     +-----------+
  | String /  |     | Keyword / |     | Separator/|     |Identifier |     | Numeric   |
  | Literal   |     | Operator  |     |Parentheses|     |   DFA     |     |   DFA     |
  | Check     |     | Lookups   |     | Lookups   |     | (State 0-2|     |(State 0-4)|
  +-----------+     +-----------+     +-----------+     +-----------+     +-----------+
        |                 |                 |                 |                 |
        +-----------------+-----------------+-----------------+-----------------+
                                                  |
                                                  v
                                      +-----------------------+
                                      | fprintf formatted tag |
                                      |     to output2.txt    |
                                      +-----------------------+

```

---

## 6. Sample Output Verification

### Step 1 Output (`output1.txt`):

```text
#include < bits / stdc + + .h > using namespace std ; int main ( ) { int x = 5 ; x += 10 ; string stringA = "String containing a literal double slash // and a /* fake comment */ block" ; string stringB = "String containing escaped tokens: \" \\\" \\t \\n and an unescaped space   " ; int y = 50 ; float z = 100.0 ; cout << "Check path: C:\\Users\\Name\\Documents\\file.txt" << endl ; int result = x / y ; int fractional = y / z ; cout << "End of the input file." << endl ; return 0 ; }

```

### Step 2 Output (`output2.txt`):

```text
[kw #include] [op <] [id bits] [op /] [id stdc] [op +] [op +] [unkn .h] [op >] [kw using] [kw namespace] [id std] [sep ;] [kw int] [id main] [par (] [par )] [par {] [kw int] [id x] [op =] [num 5] [sep ;] [id x] [op +=] [num 10] [sep ;] [kw string] [id stringA] [op =] [str "String containing a literal double slash // and a /* fake comment */ block"] [sep ;] [kw string] [id stringB] [op =] [str "String containing escaped tokens: \" \\\" \\t \\n and an unescaped space "] [sep ;] [kw int] [id y] [op =] [num 50] [sep ;] [kw float] [id z] [op =] [num 100.0] [sep ;] [kw cout] [op <<] [str "Check path: C:\\Users\\Name\\Documents\\file.txt"] [op <<] [id endl] [sep ;] [kw int] [id result] [op =] [id x] [op /] [id y] [sep ;] [kw int] [id fractional] [op =] [id y] [op /] [id z] [sep ;] [kw cout] [op <<] [str "End of the input file."] [op <<] [id endl] [sep ;] [kw return] [num 0] [sep ;] [par }]

```

---

## 7. Conclusion & Authorship

### Conclusion

The modular two-step lexer implementation successfully separates attached lexemes into space-isolated units and performs accurate token classification using Deterministic Finite Automata. By incorporating explicit escape tracking and complete string reassembly, complex string literals with backslashes (`\"`) are preserved without corrupting or swallowing downstream code blocks.

### Authorship

- **Implementation & Design:** Sadia Sultana
- **Repository:** FLC Design Lab 2026
- **Language/Standard:** C++17 (GCC)
