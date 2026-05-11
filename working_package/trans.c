#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FILE_NAME "credit.dat"
#define TEXT_EXPORT_PATH "accounts.txt"
#define TEXT_SORTED_PATH "accounts_sorted.txt"
#define MAX_ACCOUNTS 100

/* String field sizes (include space for '\0') */
#define LAST_NAME_LEN 15
#define FIRST_NAME_LEN 10

struct client_data {
    unsigned int acctNum;
    char lastName[LAST_NAME_LEN];
    char firstName[FIRST_NAME_LEN];
    double balance;
};

/* ---------- Function prototypes ---------- */
unsigned int enter_choice(void);
FILE *open_data_file(const char *path);
void initialize_file(FILE *fPtr);
void validate_file(FILE *fPtr);
int seek_to_record(FILE *fPtr, unsigned int account);
int rewind_write_record(FILE *fPtr);

void clear_input_line(void);
unsigned int get_account_number(const char *prompt);
double get_double(const char *prompt);
double get_positive_double(const char *prompt);

void text_file(FILE *readPtr);
void sync_accounts_txt(FILE *fPtr);
void write_sorted_text_file(FILE *readPtr);
void update_record(FILE *fPtr);
void new_record(FILE *fPtr);
void delete_record(FILE *fPtr);
void list_accounts(FILE *fPtr);
void display_account(FILE *fPtr);
void debit_transaction(FILE *fPtr);

static int qsort_compare_client_data(const void *a, const void *b);
static int stdin_broken(void);
static void print_account_header(FILE *out);
static void print_account_row(FILE *out, const struct client_data *c);
static int export_accounts_text_file(FILE *dataPtr, const char *path, int verbose);

/* ---------- main ---------- */
int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    FILE *cfPtr = open_data_file(FILE_NAME);
    if (cfPtr == NULL) {
        return EXIT_FAILURE;
    }

    sync_accounts_txt(cfPtr);

    unsigned int choice;
    while ((choice = enter_choice()) != 9) {
        switch (choice) {
            case 1:
                text_file(cfPtr);
                break;
            case 2:
                update_record(cfPtr);
                break;
            case 3:
                new_record(cfPtr);
                break;
            case 4:
                delete_record(cfPtr);
                break;
            case 5:
                list_accounts(cfPtr);
                break;
            case 6:
                display_account(cfPtr);
                break;
            case 7:
                debit_transaction(cfPtr);
                break;
            case 8:
                write_sorted_text_file(cfPtr);
                break;
            default:
                printf("Invalid choice\n");
                break;
        }
    }

    if (fclose(cfPtr) != 0) {
        perror("Error closing data file");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/* ---------- Input helpers ---------- */
static int stdin_broken(void) {
    return feof(stdin) != 0 || ferror(stdin) != 0;
}

void clear_input_line(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        /* discard rest of line */
    }
}

unsigned int get_account_number(const char *prompt) {
    unsigned int account;

    for (;;) {
        fflush(stdout);
        printf("%s", prompt);
        if (scanf("%u", &account) == 1) {
            clear_input_line();
            if (account >= 1 && account <= MAX_ACCOUNTS) {
                return account;
            }
            printf("Account number must be between 1 and %d.\n", MAX_ACCOUNTS);
        } else {
            if (stdin_broken()) {
                fprintf(stderr, "\nInput ended or read error; cannot continue.\n");
                exit(EXIT_FAILURE);
            }
            printf("Invalid input. Please enter a whole number.\n");
            clear_input_line();
        }
    }
}

double get_double(const char *prompt) {
    double value;

    for (;;) {
        fflush(stdout);
        printf("%s", prompt);
        if (scanf("%lf", &value) == 1) {
            clear_input_line();
            return value;
        }
        if (stdin_broken()) {
            fprintf(stderr, "\nInput ended or read error; cannot continue.\n");
            exit(EXIT_FAILURE);
        }
        printf("Invalid input. Please enter a number.\n");
        clear_input_line();
    }
}

double get_positive_double(const char *prompt) {
    for (;;) {
        double value = get_double(prompt);
        if (value > 0.0) {
            return value;
        }
        printf("Amount must be positive.\n");
    }
}

/* ---------- File open / init ---------- */
FILE *open_data_file(const char *path) {
    FILE *fPtr = fopen(path, "rb+");

    if (fPtr == NULL) {
        fPtr = fopen(path, "wb+");
        if (fPtr == NULL) {
            perror("File creation failed");
            return NULL;
        }
        initialize_file(fPtr);
    } else {
        validate_file(fPtr);
    }

    return fPtr;
}

void initialize_file(FILE *fPtr) {
    struct client_data blank = {0, "", "", 0.0};

    rewind(fPtr);
    for (int i = 0; i < MAX_ACCOUNTS; i++) {
        if (fwrite(&blank, sizeof(blank), 1, fPtr) != 1) {
            perror("Failed to initialize data file");
            break;
        }
    }
    fflush(fPtr);
}

void validate_file(FILE *fPtr) {
    if (fseek(fPtr, 0, SEEK_END) != 0) {
        perror("Failed to validate data file");
        return;
    }

    long size = ftell(fPtr);
    if (size != (long)(MAX_ACCOUNTS * sizeof(struct client_data))) {
        printf("File corrupted or wrong size. Reinitializing...\n");
        initialize_file(fPtr);
    }
    rewind(fPtr);
}

/* ---------- Seek helpers ---------- */
int seek_to_record(FILE *fPtr, unsigned int account) {
    if (account < 1 || account > MAX_ACCOUNTS) {
        printf("Invalid account number\n");
        return 0;
    }

    if (fseek(fPtr, (long)((account - 1) * sizeof(struct client_data)), SEEK_SET) != 0) {
        perror("Seek failed");
        return 0;
    }
    return 1;
}

static void print_account_header(FILE *out) {
    fprintf(out, "%-10s%-20s%-14s%14s\n",
            "Acct No.", "Last name", "First name", "Balance");
}

static void print_account_row(FILE *out, const struct client_data *c) {
    fprintf(out, "%-10u%-20s%-14s%14.2f\n",
            c->acctNum, c->lastName, c->firstName, c->balance);
}

static int export_accounts_text_file(FILE *dataPtr, const char *path, int verbose) {
    FILE *w = fopen(path, "w");
    if (w == NULL) {
        perror(path);
        return 0;
    }

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char tbuf[80];

    if (tm_info != NULL) {
        strftime(tbuf, sizeof tbuf, "%Y-%m-%d %H:%M:%S (local)", tm_info);
    } else {
        strncpy(tbuf, "(time unavailable)", sizeof tbuf);
        tbuf[sizeof tbuf - 1] = '\0';
    }

    fprintf(w, "======================================================================\n");
    fprintf(w, "  ACCOUNT REGISTER — text export from binary data: %s\n", FILE_NAME);
    fprintf(w, "======================================================================\n");
    fprintf(w, "  Each data row: account number | last name | first name | balance\n");
    fprintf(w, "  Generated: %s\n", tbuf);
    fprintf(w, "----------------------------------------------------------------------\n\n");

    struct client_data client;
    rewind(dataPtr);
    print_account_header(w);

    size_t count = 0;
    double total_balance = 0.0;

    while (fread(&client, sizeof(client), 1, dataPtr) == 1) {
        if (client.acctNum != 0) {
            print_account_row(w, &client);
            count++;
            total_balance += client.balance;
        }
    }

    if (ferror(dataPtr)) {
        perror("Error reading data file while exporting");
    }

    fprintf(w, "\n----------------------------------------------------------------------\n");
    fprintf(w, "Total active accounts: %zu\n", count);
    fprintf(w, "Aggregate balance:    %18.2f\n", total_balance);
    fprintf(w, "======================================================================\n");

    if (fclose(w) != 0) {
        perror("Error closing export file");
        return 0;
    }

    if (verbose) {
        printf("%s updated (%zu account(s); aggregate balance %.2f).\n",
               path, count, total_balance);
    }
    return 1;
}

void sync_accounts_txt(FILE *fPtr) {
    (void)export_accounts_text_file(fPtr, TEXT_EXPORT_PATH, 0);
}

/* After fread at a record, move back to the start of that record for fwrite */
int rewind_write_record(FILE *fPtr) {
    if (fseek(fPtr, -(long)sizeof(struct client_data), SEEK_CUR) != 0) {
        perror("Seek failed");
        return 0;
    }
    return 1;
}

/* ---------- Text export ---------- */
void text_file(FILE *readPtr) {
    (void)export_accounts_text_file(readPtr, TEXT_EXPORT_PATH, 1);
}

static int qsort_compare_client_data(const void *a, const void *b) {
    const struct client_data *ca = (const struct client_data *)a;
    const struct client_data *cb = (const struct client_data *)b;
    int cmp = strcmp(ca->lastName, cb->lastName);
    if (cmp != 0) {
        return cmp;
    }
    return strcmp(ca->firstName, cb->firstName);
}

void write_sorted_text_file(FILE *readPtr) {
    struct client_data clients[MAX_ACCOUNTS];
    struct client_data client;
    size_t n = 0;

    rewind(readPtr);
    while (fread(&client, sizeof(client), 1, readPtr) == 1) {
        if (client.acctNum != 0 && n < MAX_ACCOUNTS) {
            clients[n++] = client;
        }
    }

    if (ferror(readPtr)) {
        perror("Error reading data file for sorting");
        return;
    }

    if (n > 0) {
        qsort(clients, n, sizeof(clients[0]), qsort_compare_client_data);
    }

    FILE *writePtr = fopen(TEXT_SORTED_PATH, "w");
    if (writePtr == NULL) {
        perror("Sorted text file error");
        return;
    }

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char tbuf[80];
    if (tm_info != NULL) {
        strftime(tbuf, sizeof tbuf, "%Y-%m-%d %H:%M:%S (local)", tm_info);
    } else {
        strncpy(tbuf, "(time unavailable)", sizeof tbuf);
        tbuf[sizeof tbuf - 1] = '\0';
    }

    fprintf(writePtr, "======================================================================\n");
    fprintf(writePtr, "  SORTED ACCOUNTS (last name, then first name) from %s\n", FILE_NAME);
    fprintf(writePtr, "  Generated: %s  |  Active records: %zu\n", tbuf, n);
    fprintf(writePtr, "======================================================================\n\n");

    print_account_header(writePtr);

    for (size_t i = 0; i < n; i++) {
        print_account_row(writePtr, &clients[i]);
    }

    fprintf(writePtr, "\n======================================================================\n");

    if (fclose(writePtr) != 0) {
        perror("Error closing sorted export file");
        return;
    }
    printf("%s generated successfully (%zu row(s)).\n", TEXT_SORTED_PATH, n);
}

/* ---------- Add record ---------- */
void new_record(FILE *fPtr) {
    struct client_data client = {0};
    unsigned int account = get_account_number("Enter account number (1-100): ");

    if (!seek_to_record(fPtr, account)) {
        return;
    }

    if (fread(&client, sizeof(client), 1, fPtr) != 1) {
        puts("Error reading the record.");
        return;
    }

    if (client.acctNum != 0) {
        printf("Account %u already exists\n", account);
        return;
    }

    {
        char scan_fmt[48];
        snprintf(scan_fmt, sizeof scan_fmt,
                   "%%%ds %%%ds %%lf",
                   LAST_NAME_LEN - 1,
                   FIRST_NAME_LEN - 1);

        fflush(stdout);
        printf("Enter lastname firstname balance: ");
        if (scanf(scan_fmt,
                  client.lastName,
                  client.firstName,
                  &client.balance) != 3) {
            if (stdin_broken()) {
                fprintf(stderr, "\nInput ended or read error; cannot continue.\n");
                exit(EXIT_FAILURE);
            }
            printf("Invalid input. Example: Kumar Ravi 1500.50\n");
            clear_input_line();
            return;
        }
    }
    clear_input_line();

    if (client.balance < 0.0) {
        printf("Balance cannot be negative.\n");
        return;
    }

    client.acctNum = account;

    if (!rewind_write_record(fPtr)) {
        return;
    }

    if (fwrite(&client, sizeof(client), 1, fPtr) != 1) {
        printf("Failed to save account\n");
        return;
    }
    fflush(fPtr);

    sync_accounts_txt(fPtr);
    printf("Account %u created successfully\n", account);
}

/* ---------- Update ---------- */
void update_record(FILE *fPtr) {
    struct client_data client;
    unsigned int account = get_account_number("Enter account: ");

    if (!seek_to_record(fPtr, account)) {
        return;
    }

    if (fread(&client, sizeof(client), 1, fPtr) != 1) {
        puts("Error reading the record.");
        return;
    }

    if (client.acctNum == 0) {
        printf("Account not found\n");
        return;
    }

    {
        double transaction;

        printf("Current balance: %.2f\n", client.balance);
        transaction = get_double("Enter transaction (+/-): ");

        if (client.balance + transaction < 0.0) {
            printf("Transaction would result in negative balance. Not applied.\n");
            return;
        }

        client.balance += transaction;

        if (!rewind_write_record(fPtr)) {
            return;
        }

        if (fwrite(&client, sizeof(client), 1, fPtr) != 1) {
            printf("Failed to update account\n");
            return;
        }
        fflush(fPtr);

        sync_accounts_txt(fPtr);
        printf("Updated balance: %.2f\n", client.balance);
    }
}

/* ---------- Delete ---------- */
void delete_record(FILE *fPtr) {
    struct client_data client;
    struct client_data blank = {0};
    unsigned int account = get_account_number("Enter account to delete: ");

    if (!seek_to_record(fPtr, account)) {
        return;
    }

    if (fread(&client, sizeof(client), 1, fPtr) != 1) {
        puts("Error reading the record.");
        return;
    }

    if (client.acctNum == 0) {
        printf("Account does not exist\n");
        return;
    }

    if (!rewind_write_record(fPtr)) {
        return;
    }

    if (fwrite(&blank, sizeof(blank), 1, fPtr) != 1) {
        printf("Failed to delete account\n");
        return;
    }
    fflush(fPtr);

    sync_accounts_txt(fPtr);
    printf("Account deleted\n");
}

/* ---------- List ---------- */
void list_accounts(FILE *fPtr) {
    struct client_data client;
    int count = 0;
    double total_balance = 0.0;

    rewind(fPtr);
    printf("\n");
    print_account_header(stdout);

    while (fread(&client, sizeof(client), 1, fPtr) == 1) {
        if (client.acctNum != 0) {
            print_account_row(stdout, &client);
            count++;
            total_balance += client.balance;
        }
    }

    if (ferror(fPtr)) {
        perror("Error reading data file while listing");
    }

    printf("\nTotal active accounts: %d\n", count);
    printf("Total balance across all accounts: %.2f\n", total_balance);
}

/* ---------- Display one account ---------- */
void display_account(FILE *fPtr) {
    unsigned int account = get_account_number("Enter account number to display: ");

    if (!seek_to_record(fPtr, account)) {
        return;
    }

    struct client_data client;
    if (fread(&client, sizeof(client), 1, fPtr) != 1) {
        puts("Error reading the record.");
        return;
    }

    if (client.acctNum == 0) {
        printf("No information for account slot %u (empty).\n", account);
        return;
    }

    printf("\n");
    print_account_header(stdout);
    print_account_row(stdout, &client);
}

/* ---------- Debit (ATM-style) ---------- */
void debit_transaction(FILE *fPtr) {
    unsigned int account = get_account_number("Enter account number: ");
    double amount = get_positive_double("Enter debit amount: ");

    if (!seek_to_record(fPtr, account)) {
        return;
    }

    struct client_data client;
    if (fread(&client, sizeof(client), 1, fPtr) != 1) {
        puts("Error reading the record.");
        return;
    }

    if (client.acctNum == 0) {
        printf("Account does not exist.\n");
        return;
    }

    if (client.balance < amount) {
        printf("Insufficient funds. Balance: %.2f\n", client.balance);
        return;
    }

    client.balance -= amount;

    if (!rewind_write_record(fPtr)) {
        return;
    }

    if (fwrite(&client, sizeof(client), 1, fPtr) != 1) {
        printf("Failed to save account after debit\n");
        return;
    }
    fflush(fPtr);

    sync_accounts_txt(fPtr);
    printf("Debit successful. New balance: %.2f\n", client.balance);
}

/* ---------- Menu ---------- */
unsigned int enter_choice(void) {
    unsigned int choice;

    printf("\n1 - Regenerate " TEXT_EXPORT_PATH " (also auto-updated after changes)\n"
           "2 - Update account\n"
           "3 - Add account\n"
           "4 - Delete account\n"
           "5 - List all accounts\n"
           "6 - Search account and display\n"
           "7 - Debit transaction (withdraw)\n"
           "8 - Write sorted text file (" TEXT_SORTED_PATH ")\n"
           "9 - End program\n"
           "Choice: ");

    fflush(stdout);
    if (scanf("%u", &choice) != 1) {
        if (stdin_broken()) {
            fprintf(stderr, "\nInput ended or read error; exiting menu.\n");
            exit(EXIT_FAILURE);
        }
        printf("Invalid menu choice\n");
        clear_input_line();
        return 0;
    }
    clear_input_line();
    return choice;
}
