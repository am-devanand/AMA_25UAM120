# Mini Project Changes

This file explains the improvements made to complete the assignment goals in a beginner-friendly way.

## Goals Completed

### 1) Compile and run the code
- Compiled successfully using:
  - `gcc -Wall -Wextra -std=c11 trans.c -o trans`
- Program runs correctly with the menu options.

### 2) Perform testing
- Tested full flow:
  - Add account
  - Update account balance
  - List accounts
  - Generate `accounts.txt`
- The tested account data was saved and displayed correctly.

### 3) Add account and regenerate `accounts.txt`
- Added account number `1`:
  - Last name: `Kumar`
  - First name: `Ravi`
  - Balance: `1000`
- Updated balance by `+250`, final balance became `1250`.
- Generated `accounts.txt` and verified correct output.

### 4) Identify and fix logical errors
- Added safer input handling for menu and account number input.
- Added input buffer cleanup to avoid bad input breaking later reads.
- Added checks for file read/write failures in add/update/delete functions.
- Added clear messages for invalid input and failed operations.

### 5) Add new functionality
- Improved `List accounts` option:
  - Shows all active accounts
  - Also shows:
    - total active account count
    - total balance across all active accounts

### 6) Improve performance / efficiency
- Continued using direct record access with `fseek` (already efficient for fixed-size records).
- Kept changes simple and lightweight (no unnecessary loops or memory-heavy structures).
- Validation and I/O checks reduce chances of repeated failed operations.

## Code Design Notes (Simple and Beginner-Friendly)
- No advanced C features were added.
- Existing structure and function names were kept.
- Helper functions were small and focused:
  - `clearInputBuffer()`
  - `readUnsignedInt()`

## Files Changed
- `trans.c` (main improvements and new simple functionality)
- `changes.md` (this report)
