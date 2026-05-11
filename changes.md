# Mini Project — Full changelog (reference code → current)

This document lists **every major change** made from the **original textbook-style / Replit reference program** through the **current `trans.c`**, plus how **`accounts.txt`** works and what **`working_package/`** contains. Shorter reference sections (menu, build) follow the detailed history.

---

## Table of contents

1. [Baseline: what the original program did](#1-baseline-what-the-original-program-did)
2. [Complete change list (scratch → now)](#2-complete-change-list-scratch--now)
3. [What the program does today (summary)](#3-what-the-program-does-today-summary)
4. [Output files and `accounts.txt`](#4-output-files-and-accountstxt)
5. [Menu reference](#5-menu-reference)
6. [Repository layout](#6-repository-layout)
7. [Build and test](#7-build-and-test)
8. [Demo talking points](#8-demo-talking-points)

---

## 1. Baseline: what the original program did

The starting point (typical Deitel-style **transaction processing** example, e.g. [Replit reference](https://replit.com/@ashokb/Unit5Programs#trans.c)) behaved roughly as follows:

| Aspect | Original behavior |
|--------|---------------------|
| **Data file** | Binary **`credit.dat`**, treated as an array of **100** fixed-size records. |
| **Record layout** | Account number, last name, first name, balance (C `struct`, often named like `clientData`). |
| **Indexing** | Account **n** (1-based) at byte offset **`(n - 1) × sizeof(struct …)`**; **`fseek`** + **`fread`** / **`fwrite`**. |
| **Empty slot** | **`acctNum == 0`** meaning “unused record.” |
| **Open mode** | **`fopen("credit.dat", "rb+")`** only — if the file **did not exist**, the program failed instead of creating it. |
| **Uninitialized file** | No guarantee of 100 initialized slots on first run; random access could read **garbage** or fail expectations. |
| **Menu** | Fewer options; **“end program”** was typically the **last** small integer (e.g. **5**), not **9**. |
| **`accounts.txt`** | Produced by scanning the binary file and writing text (often **`accounts.txt`**). |
| **`textFile` / export loop** | Often written as **`while (!feof(f)) { fread(...); … }`** — a **classic bug**: **`feof`** becomes true only **after** a failed read, so the loop can run **one extra time** (duplicate last line or garbage). |
| **Input** | **`scanf`** without always clearing the rest of the line on failure → user could get **stuck** repeating invalid input. |
| **Types vs format** | **`scanf("%d", &unsigned_var)`** — undefined behavior; **`%u`** is required for **`unsigned int`**. |
| **I/O robustness** | **`fread`** / **`fwrite`** return values often ignored; **`fflush`** after writes rarely used. |
| **`main`** | Often omitted an explicit **`return 0;`** / status code. |
| **Naming** | Mixed **camelCase** (`updateRecord`, `textFile`) and **`struct clientData`**. |

That baseline is the **“from scratch”** reference; everything in [section 2](#2-complete-change-list-scratch--now) is what was **changed or added** on top of it.

---

## 2. Complete change list (scratch → now)

Below is a **single consolidated list** of improvements, in logical groups (not every Git commit). Together they describe the path from the original code to the current project.

### 2.1 File lifecycle: create, validate, initialize

| Change | Why |
|--------|-----|
| **`open_data_file(path)`** | Try **`rb+`** first; if missing, **`wb+`** and create the database instead of exiting immediately. |
| **`initialize_file`** | Write **100** blank records (`acctNum == 0`, empty names, balance **0.0**) so every slot exists and is predictable. |
| **`validate_file`** | **`fseek`** to end, **`ftell`** size; if size ≠ **`100 × sizeof(struct client_data)`**, treat as corrupt/wrong and **reinitialize**, then **`rewind`**. |
| **`fflush`** after initializing / writing records | Reduces risk of buffered data not appearing on disk during demos or abrupt exit. |

### 2.2 Menu and features (functionality)

| Change | Why |
|--------|-----|
| **Option 5 — list all accounts** (`**list_accounts**`) | Meets “list all account information” style requirements; shows **count** and **total balance** across active accounts. |
| **Option 6 — search / display one account** (`**display_account**`) | Demonstrates **random read** only: **`fseek`**, **`fread`**, clear message if slot is empty. |
| **Option 7 — debit / withdraw** (`**debit_transaction**`) | Faculty-style “ATM debit” story: positive amount, account must exist, **no overdraft** (`balance >= amount`). |
| **Option 8 — sorted text export** (`**write_sorted_text_file**` → **`accounts_sorted.txt`**) | “Sort records” requirement: load non-empty records, **`qsort`** by **last name**, then **first name**; comparator is **file-scope** (standard C, Windows-friendly), not a nested function. |
| **Exit moved to option 9** | Extra features inserted as options **5–8**; **9** ends the program. |
| **Option 1** | Regenerates **`accounts.txt`**; label updated to note it is **also auto-synced** after data changes and at startup (see [section 4](#4-output-files-and-accountstxt)). |

### 2.3 Correctness: loops, formats, seeks

| Change | Why |
|--------|-----|
| **Replace `while (!feof(...))` + `fread`** with **`while (fread(...) == 1)`** everywhere records are scanned | Only process a row when a full record was actually read; avoids **duplicate last row** / garbage. |
| **`scanf`** uses **`%u`** for **`unsigned int`** (menu choice, account numbers where applicable) | Avoids **undefined behavior** from **`%d`** with an **`unsigned int *`**. |
| **`printf`** for unsigned accounts uses **`%u`** where appropriate | Consistent with type. |
| **Backward seek after `fread` before `fwrite`** | Implemented as **`rewind_write_record`**: **`fseek(fp, -(long)sizeof(struct client_data), SEEK_CUR)`** with **return check** — avoids **mixed sign** bugs on some platforms and satisfies stricter compilers. |
| **`update_record`**: `double transaction` scoped in a block after confirming account exists | Avoids “mixed declarations and code” issues on older / stricter C compilers (e.g. some Windows toolchains). |

### 2.4 Input safety (beginner- and demo-friendly)

| Change | Why |
|--------|-----|
| **`clear_input_line`** | After a bad **`scanf`**, read until newline or EOF so the next prompt does not see stale characters (**no infinite invalid loop**). |
| **`get_account_number`** | Re-prompt until input is a valid slot **1 … 100** — prevents bad **`fseek`** targets. |
| **`get_double`** / **`get_positive_double`** | Reliable numeric input for transactions and debits; positive-only for debit amount. |
| **`fflush(stdout)`** before prompts that use **`scanf`** | Helps when **`stdout`** is fully buffered (pipes, some IDEs). |
| **`stdin_broken`** (`**feof**` / **`ferror**` on stdin) | If input is a **closed pipe** or broken stream, **`exit(EXIT_FAILURE)`** instead of spinning forever (useful for **`make demo`** scripts). |

### 2.5 Read/write error handling

| Change | Why |
|--------|-----|
| Check **`fread(...) == 1`** after reading a single record; on failure print a message and **return** | Avoids using partially read / garbage data. |
| Check **`fwrite`** return values | Detect disk full / I/O errors. |
| **`ferror`** after long **`fread`** loops (export, list, sorted read path) | Surfaces read errors instead of silent truncation. |
| **`fclose`** return value checked | On main database file and on text export files. |

### 2.6 Naming, structure, and `main`

| Change | Why |
|--------|-----|
| **`struct client_data`** (was-style `clientData`) | Clearer, conventional snake-style type name. |
| **Snake_case function names** (`**update_record**`, `**text_file**`, …) | Consistent style and easier grep/read. |
| **`main(int argc, char *argv[])`** with **`(void)argc; (void)argv;`** | Silences unused-parameter warnings under **`-Wall -Wextra`**. |
| **`return EXIT_SUCCESS`** / **`EXIT_FAILURE`** | Proper process exit status; failure if database **`fclose`** fails. |

### 2.7 Business rules (domain logic)

| Change | Why |
|--------|-----|
| **Update transaction** cannot drive **balance &lt; 0** | “No negative balance” policy. |
| **New account** cannot have **negative opening balance** | Same policy at creation. |
| **Debit** requires **amount &gt; 0** and **`balance >= amount`** | ATM-like withdraw without overdraft. |

### 2.8 Text export, columns, and auto-sync (`accounts.txt`)

| Change | Why |
|--------|-----|
| **`print_account_header`** / **`print_account_row`** | **Single place** for column widths and labels; used by **terminal list**, **single-account display**, **`accounts.txt`**, and sorted export body. |
| **Wider columns** (e.g. **Acct No.**, **Last name**, **First name**, **Balance**) | Easier to read in editors and when names are long. |
| **`TEXT_EXPORT_PATH`** / **`TEXT_SORTED_PATH`** macros | One place to change filenames. |
| **`#include <time.h>`** and **`strftime`** in exports | **Timestamp** on human-readable exports for lab reports. |
| **Banner + footer** on **`accounts.txt`** | Explains source file, column meaning, **count of active accounts**, **aggregate balance**. |
| **`export_accounts_text_file`** + **`sync_accounts_txt`** | Central implementation; **`sync_accounts_txt`** runs **silently** after startup and after successful **add / update / delete / debit** so **`accounts.txt`** stays aligned with **`credit.dat`**. |
| **`text_file` (menu 1)** | Calls the same exporter with **verbose** terminal summary. |
| **Sorted file (`accounts_sorted.txt`)** | Extra banner lines (title, time, **record count**); same row printers for consistency. |

### 2.9 Field sizes and `scanf` for new accounts

| Change | Why |
|--------|-----|
| **`LAST_NAME_LEN`**, **`FIRST_NAME_LEN`** in the `struct` | Avoid magic numbers in declarations. |
| **`snprintf`** into a format string for **`scanf`** in **`new_record`** | Max token widths stay **in sync** with array sizes (**`N-1`** for **`%s`**). |

### 2.10 Repository, docs, and runnable bundle

| Change | Why |
|--------|-----|
| **`working_package/`** | Self-contained **Makefile**, **`demo_seed.txt`**, **`trans.c`**, **`README.md`**, sample outputs — easy zip/demo without hunting paths. |
| **`make demo`** | Clean **`credit.dat`** / exports, run **`trans`** non-interactively, produce a known-good **`accounts.txt`**. |
| **Root `README.md`** | Points to **`working_package/`** and this changelog. |
| **`.gitignore`** | Ignores built **`trans`** binaries (root and package); avoids committing executables. |
| **Remote `origin`** | Pushes target **[`am-devanand/AMA_25UAM120`](https://github.com/am-devanand/AMA_25UAM120)** (folder/repo rename on GitHub); **`trans` binary removed from Git** — rebuild with **`gcc`** or **`make`**. |

---

## 3. What the program does today (summary)

- **100** records in **`credit.dat`**; **`struct client_data`**; empty if **`acctNum == 0`**.
- **Random access** via **`fseek`** using **`(slot - 1) × sizeof(struct client_data)`**.
- **Menu 1–9** as in [section 5](#5-menu-reference); **`accounts.txt`** export with **sync** as in [section 4](#4-output-files-and-accountstxt).

---

## 4. Output files and `accounts.txt`

### `accounts.txt`

- **Banner**: states that the export comes from **`credit.dat`**, describes columns.
- **Timestamp**: local time via **`strftime`**.
- **Table**: **Acct No.**, **Last name**, **First name**, **Balance** (fixed-width columns).
- **Footer**: **total active accounts**, **aggregate balance**.

**Updated when:** (1) program start, after **`credit.dat`** opens; (2) after each successful **add / update / delete / debit**; (3) menu **1** (with a short confirmation line).

### `accounts_sorted.txt`

- Written only when the user chooses menu **8**; sorted by **last name**, then **first name** (**`qsort`**); includes its own header block with time and row count.

---

## 5. Menu reference

| # | Action |
|---|--------|
| **1** | Regenerate **`accounts.txt`** (also auto-updated after changes and at startup). |
| **2** | Update balance; cannot go **negative**. |
| **3** | Add account; opening balance cannot be **negative**. |
| **4** | Delete account (blank record). |
| **5** | List all active accounts + totals. |
| **6** | Display one account by number. |
| **7** | Debit (withdraw); **positive** amount; **no overdraft**. |
| **8** | Write **`accounts_sorted.txt`**. |
| **9** | Exit. |

---

## 6. Repository layout

| Path | Role |
|------|------|
| `trans.c` | All program logic (source of truth). |
| `changes.md` | This full changelog + reference sections. |
| `README.md` | Course brief + pointer to **`working_package/`**. |
| `working_package/` | Makefile, demo seed, copy of `trans.c`, local README, optional sample `accounts.txt` / `credit.dat` from demo. |
| `.gitignore` | Build artifacts (`trans`, `*.o`). |

---

## 7. Build and test

**Root**

```bash
gcc -Wall -Wextra -std=c11 trans.c -o trans
./trans
```

**`working_package/`**

```bash
cd working_package
make
make demo
```

---

## 8. Demo talking points

- **Random access:** “Slot **n** is at offset **(n−1) × record size**; I **`fseek`** then **`fread`**/**`fwrite`**.”
- **Empty slot:** “**`acctNum == 0`** means no customer in that slot.”
- **`feof` bug:** “**`feof`** is true only after you read past EOF, so a **`while (!feof)`** loop can run one time too many; I loop while **`fread == 1`**.”
- **`accounts.txt`:** “It mirrors **`credit.dat`** with account number, names, balance, time, and totals; it refreshes on start and after each change.”
- **Input:** “If **`scanf`** fails, I clear the line and ask again; if stdin is closed, I exit instead of looping.”

---

*This file is the authoritative “what changed from the original mini project code” narrative for grading and demos.*
