# Working package — Transaction processing mini project

This folder is a **self-contained copy** of the assignment program: same `trans.c` as the repository root, plus build rules and a scripted demo so you always get a working `credit.dat`, `accounts.txt`, and a runnable binary without hunting files across the tree.

## Contents

| File | Purpose |
|------|---------|
| `trans.c` | Full source (kept in sync with the repo root; build from here or from `..`). |
| `Makefile` | `make` / `make clean` / `make demo` / `make run`. |
| `demo_seed.txt` | Non-interactive input: creates three sample accounts, then exits. |
| `README.md` | This quick start. |
| `changes.md` | Detailed changelog and design notes (kept in sync with the repo root `changes.md`). |

After `make demo`, you will also have:

| Generated | Purpose |
|-----------|---------|
| `trans` | Executable. |
| `credit.dat` | Binary random-access store (100 fixed slots). |
| `accounts.txt` | Human-readable export: **account number**, names, balance, timestamp, totals. |
| `accounts_sorted.txt` | Only after you run menu option **8** in an interactive session (not created by `demo_seed.txt`). |

## Build

```bash
cd working_package
make
```

## Populate sample data and `accounts.txt`

```bash
make demo
```

Then open `accounts.txt` in any editor: it lists **Acct No.**, **Last name**, **First name**, **Balance**, plus a footer with **how many accounts** are active and the **aggregate balance**.

## Interactive use

```bash
make run
```

Choose **1** at any time to regenerate `accounts.txt` from `credit.dat` (with a short summary on the terminal). The program also **refreshes `accounts.txt` automatically** after add, update, delete, and debit, and once when the program starts.

## Keeping `trans.c` in sync

Edits are normally made in the repository root `trans.c`. After you change it, refresh this folder with:

```bash
cp ../trans.c .
```

Then `make` or `make demo` again.

## Full lab brief

See the parent directory `README.md` for the official 24UCS271 objectives, rubric, and references.
