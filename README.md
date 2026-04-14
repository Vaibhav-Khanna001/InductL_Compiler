# InductL

**An inductive programming language where the contract is the syntax.**

---

## Overview

In most programming languages, what a function is *supposed to do* and *how it does it* are entirely separate concerns. Documentation, comments, and tests attempt to bridge that gap — but they live outside the code itself.

InductL eliminates that gap. Contracts are not annotations or tests bolted on after the fact — they are the syntax. The compiler validates your logic against your stated guarantees at compile time, before the program ever runs.

---

## How It Works

InductL introduces `ensure` statements as first-class syntax. When you declare what a function must guarantee, the compiler uses that declaration as a constraint — not just a hint.

**If the logic inside a function contradicts its `ensure` statement, the compiler raises a Logic Violation error and halts.**

```inductL
fn transfer(sender: Account, receiver: Account, amount: Float) -> (Account, Account)
  ensure sender.balance >= 0
  ensure receiver.balance >= 0
{
  sender.balance  -= amount
  receiver.balance += amount
  return (sender, receiver)
}
```

In the example above, if a code path exists where `sender.balance` could go negative, the compiler catches it — no runtime, no tests required.

---

## Key Features

- **Compile-time contract validation** — Logic violations are caught before execution, not during.
- **`ensure` as syntax** — Postconditions are a structural part of function definitions, not comments or decorators.
- **Logic Violation errors** — A dedicated error class that clearly distinguishes contract failures from ordinary bugs.
- **Designed for edge-case thinking** — The language encourages you to define what *cannot* happen before you write what *does* happen.

---

## Use Cases

**Fintech & Banking**
Guarantee that transfer functions never produce a negative balance. Write the constraint once — the compiler enforces it everywhere.

**Embedded Systems**
Ensure a motor's speed never exceeds a safety threshold. Catch constraint violations at build time, before deployment to hardware.

**Academic Learning**
Teach students to reason about edge cases — like division by zero or overflow — before writing a single line of logic. The compiler becomes a learning tool.

---

## Why InductL?

| | Traditional Languages | InductL |
|---|---|---|
| Contracts | Comments, docs, or external tests | First-class syntax |
| Validation | Runtime or manual testing | Compile-time |
| Edge case handling | Developer discipline | Compiler-enforced |
| Logic violations | Silent bugs or exceptions | Explicit, named error |

---


---

## Getting Started

### Prerequisites

Open **MSYS2 MinGW64** and install the required tools:

```bash
pacman -S mingw-w64-x86_64-gcc flex bison make
```

### Build

```bash
make
```

This runs the `Makefile`, which orchestrates the full compilation pipeline:

1. **Flex** processes the `.l` lexer file → generates a C tokenizer (`lex.yy.c`)
2. **Bison** processes the `.y` grammar file → generates a C parser (`parser.tab.c` + `parser.tab.h`)
3. **g++** compiles all generated C/C++ files → produces the `inductl` executable

### Run

```bash
./inductl <your_file>
```

**Example:**

```bash
./inductl test_file.inductL
```

The compiler will parse your file, validate all `ensure` contracts against the function logic, and either produce output or raise a **Logic Violation** error if a contract is breached.

---

## Under the Hood

InductL's compiler is built on a classical compiler pipeline extended with a contract validation phase.

```
Source File (.inductL)
        │
        ▼
  ┌───────────┐
  │   Lexer   │  (Flex)   — Tokenizes raw source into symbols
  └─────┬─────┘
        │
        ▼
  ┌───────────┐
  │  Parser   │  (Bison)  — Builds an Abstract Syntax Tree (AST)
  └─────┬─────┘
        │
        ▼
  ┌──────────────────┐
  │ Contract Checker │  (Custom) — Validates ensure statements against logic
  └─────┬────────────┘
        │
   ┌────┴─────┐
   ▼          ▼
Output    Logic Violation Error
```

**Lexer (Flex)**
Reads the raw source file character by character and converts it into a stream of tokens — keywords like `fn` and `ensure`, identifiers, operators, and literals.

**Parser (Bison)**
Consumes the token stream and applies the grammar rules of InductL to build an Abstract Syntax Tree (AST) — a structured, in-memory representation of your program.

**Contract Checker**
The core innovation. Walks the AST and, for each function, checks whether any execution path can violate the `ensure` postconditions. If a contradiction is found, it raises a **Logic Violation** error and halts — no executable is produced.