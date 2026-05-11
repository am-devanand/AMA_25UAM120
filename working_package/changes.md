# Mini Project — Changelog and technical notes

This document describes the **transaction processing** program, every major code change, how **`accounts.txt`** is produced, and how the **`working_package/`** folder is meant to be used for demos and submission.

---

## Table of contents

1. [What the program does](#1-what-the-program-does)
2. [Output files and `accounts.txt`](#2-output-files-and-accountstxt)
3. [Menu reference](#3-menu-reference)
4. [Assignment-aligned improvements](#4-assignment-aligned-improvements)
5. [Code quality and robustness](#5-code-quality-and-robustness)
6. [Working package folder](#6-working-package-folder)
7. [Build and test](#7-build-and-test)
8. [Demo talking points](#8-demo-talking-points)

---

## 1. What the program does

The program maintains up to **100** fixed-size records in a binary file **`credit.dat`**. Each record is a **`struct client_data`** holding:

| Field | Meaning |
|--------|--------|
| `acctNum` | Customer account number (also **0** = empty slot). |
| `lastName`, `firstName` | Fixed-length C strings. |
| `balance` | `double` balance. |

**Random access:** menu account numbers **1 … 100** map to file byte offset  
`(account_number - 1) × sizeof(struct client_data)`  
from the start of `credit.dat`. The code uses **`fseek`**, **`fread`**, and **`fwrite`**.

**Empty slots:** `acctNum == 0` means “no account here.” On first run (or if the file is missing or the wrong length), **`initialize_file`** writes **100** blank records so every slot can be read safely.

---

## 2. Output files and `accounts.txt`

### `accounts.txt` — main human-readable export

This file is the bridge between the **binary** database and something you can open in a text editor, diff in Git, or attach to a report.

**What each run writes**

- A short **banner** naming the source binary (`credit.dat`) and what each column means.
- A **timestamp** (local wall-clock time via `strftime`).
- A **table** with clear headers:
  - **Acct No.** — account number  
  - **Last name** / **First name**  
  - **Balance** — two decimal places, fixed column width  
- A **footer** with:
  - **Total active accounts** (count of rows where `acctNum != 0`)
  - **Aggregate balance** (sum of those balances)

**When it is updated**

1. **On program start** — once `credit.dat` is opened successfully, the program calls **`sync_accounts_txt`**, which rewrites `accounts.txt` from the current binary data (silent unless `fopen` fails; errors use `perror`).
2. **After every successful change** to stored data — **`new_record`**, **`update_record`**, **`delete_record`**, and **`debit_transaction`** each call **`sync_accounts_txt`** after a successful `fwrite` + `fflush`, so the text file stays aligned with `credit.dat` without forcing the user to remember menu option **1**.
3. **Menu option 1** — **`text_file`** calls the same exporter with **verbose** output on the terminal (path, row count, aggregate balance).

So **`accounts.txt` always reflects account numbers and the rest of the details** stored in `credit.dat` for every active account, plus summary lines suitable for marking or demos.

### `accounts_sorted.txt`

Created by menu **8**. Same column layout as the table above, with a **sorted** banner (sort order: **last name**, then **first name**, using **`qsort`**). This file is **not** auto-refreshed on every mutation (only when the user asks for option **8**).

---

## 3. Menu reference

| # | Action |
|---|--------|
| **1** | Regenerate **`accounts.txt`** (also auto-updated after changes and at startup). |
| **2** | Update balance (transaction); balance cannot go **negative**. |
| **3** | Add account in an empty slot; opening balance cannot be negative. |
| **4** | Delete account (blank record). |
| **5** | List all active accounts on the terminal + totals. |
| **6** | Look up one account by number. |
| **7** | Debit (withdraw); amount **> 0**; **no overdraft**. |
| **8** | Write **`accounts_sorted.txt`**. |
| **9** | Exit. |

---

## 4. Assignment-aligned improvements

- **First-run / corrupt file handling:** `open_data_file` → `rb+` or create with `wb+` + **`initialize_file`**; **`validate_file`** checks total file size.
- **Listing and totals** (menu **5**): count and sum of balances.
- **Innovation-style features:** debit transaction (**7**), sorted export (**8**), strong input validation (**`get_account_number`**, **`get_double`**, **`get_positive_double`**), negative-balance rules.
- **Classic bug fix:** record iteration uses **`while (fread(...) == 1)`**, not **`while (!feof(...))`**, so the last record is never duplicated.
- **Unsigned I/O:** **`%u`** where values are `unsigned int`.
- **Naming:** **`struct client_data`**, **snake_case** functions.
- **Exit codes:** **`EXIT_SUCCESS`** / **`EXIT_FAILURE`** from **`main`**; **`fclose`** on the database file is checked.

---

## 5. Code quality and robustness

- **Shared formatting:** **`print_account_header`** / **`print_account_row`** keep the terminal list, single-account view, and text exports visually consistent.
- **Field size macros:** **`LAST_NAME_LEN`**, **`FIRST_NAME_LEN`**; **`snprintf`** builds the **`scanf`** pattern in **`new_record`** so widths stay consistent.
- **Portable backward seek:** **`rewind_write_record`** uses **`-(long)sizeof(struct client_data)`** with **`SEEK_CUR`** and checks **`fseek`**.
- **`fflush`** after writes to `credit.dat` for predictable persistence during demos.
- **`ferror`** after long reads; **`fclose`** checked on export files.
- **`fflush(stdout)`** before interactive **`scanf`**; **`stdin_broken`** avoids infinite loops when **stdin** hits **EOF** (e.g. closed pipe).

---

## 6. Working package folder

The directory **`working_package/`** is a **portable bundle** for labs and submission:

- **`trans.c`** — same source as the repository root (copy updated whenever the main program changes).
- **`Makefile`** — `make`, `make clean`, **`make demo`** (wipes old `credit.dat` / exports, runs **`trans`** with **`demo_seed.txt`**), **`make run`** (interactive).
- **`demo_seed.txt`** — scripted session that creates **three** sample accounts and exits.
- **`README.md`** — short instructions for that folder only.

Use **`make demo`** inside **`working_package/`** to get a known-good **`accounts.txt`** with real account numbers and names without typing.

---

## 7. Build and test

**Repository root**

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

Inspect **`accounts.txt`** after **`make demo`** to verify account numbers and balances.

---

## 8. Demo talking points

- **Random access:** “Slot **n** is offset **(n−1) × record size**; I **`fseek`** there before **`fread`**/**`fwrite`**.”
- **Empty record:** “**`acctNum == 0`** means unused slot.”
- **`accounts.txt`:** “It’s regenerated from **`credit.dat`** with a timestamp and totals; it stays in sync after each add, update, delete, or debit.”
- **Read loop:** “I stop when **`fread`** returns **0**, not when **`feof`** flips, so I never print a phantom row.”

---

## Files in this repository (typical)

| Path | Role |
|------|------|
| `trans.c` | Source of truth for program logic. |
| `changes.md` | This document (root). |
| `working_package/` | Runnable bundle + demo seed (see its `README.md`). |
| `README.md` | Official course brief and rubric pointers. |
| `.gitignore` | Ignores built `trans` binaries (optional patterns for generated data). |

---

*Last expanded: documentation pass including `accounts.txt` export behavior and `working_package/` layout.*
