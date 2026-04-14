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