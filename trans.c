#include <stdio.h>
#include <stdlib.h>

#define FILE_NAME "credit.dat"
#define MAX_ACCOUNTS 100

struct clientData {
    unsigned int acctNum;
    char lastName[15];
    char firstName[10];
    double balance;
};

// ---------- Function Prototypes ----------
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void listAccounts(FILE *fPtr);
void initializeFile(FILE *fPtr);
void validateFile(FILE *fPtr);
int seekToRecord(FILE *fPtr, unsigned int account);
void clearInputBuffer(void);
int readUnsignedInt(const char *prompt, unsigned int *value);

// ---------- MAIN ----------
int main() {
    FILE *cfPtr;

    cfPtr = fopen(FILE_NAME, "rb+");

    if (cfPtr == NULL) {
        cfPtr = fopen(FILE_NAME, "wb+");
        if (cfPtr == NULL) {
            perror("File creation failed");
            exit(1);
        }
        initializeFile(cfPtr);
    } else {
        validateFile(cfPtr);
    }

    unsigned int choice;
    while ((choice = enterChoice()) != 6) {
        switch (choice) {
            case 1: textFile(cfPtr); break;
            case 2: updateRecord(cfPtr); break;
            case 3: newRecord(cfPtr); break;
            case 4: deleteRecord(cfPtr); break;
            case 5: listAccounts(cfPtr); break;
            default: printf("Invalid choice\n");
        }
    }

    fclose(cfPtr);
    return 0;
}

// ---------- INPUT HELPERS ----------
void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        /* discard remaining characters */
    }
}

int readUnsignedInt(const char *prompt, unsigned int *value) {
    if (prompt != NULL) {
        printf("%s", prompt);
    }

    if (scanf("%u", value) != 1) {
        printf("Invalid input. Please enter a number.\n");
        clearInputBuffer();
        return 0;
    }

    clearInputBuffer();
    return 1;
}

// ---------- FILE INITIALIZATION ----------
void initializeFile(FILE *fPtr) {
    struct clientData blank = {0, "", "", 0.0};
    rewind(fPtr);

    for (int i = 0; i < MAX_ACCOUNTS; i++) {
        if (fwrite(&blank, sizeof(blank), 1, fPtr) != 1) {
            perror("Failed to initialize data file");
            break;
        }
    }
    fflush(fPtr);
}

// ---------- FILE VALIDATION ----------
void validateFile(FILE *fPtr) {
    fseek(fPtr, 0, SEEK_END);
    long size = ftell(fPtr);

    if (size != MAX_ACCOUNTS * sizeof(struct clientData)) {
        printf("File corrupted. Reinitializing...\n");
        initializeFile(fPtr);
    }
}

// ---------- SEEK HELPER ----------
int seekToRecord(FILE *fPtr, unsigned int account) {
    if (account < 1 || account > MAX_ACCOUNTS) {
        printf("Invalid account number\n");
        return 0;
    }

    if (fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET) != 0) {
        perror("Seek failed");
        return 0;
    }
    return 1;
}

// ---------- CREATE TEXT FILE ----------
void textFile(FILE *readPtr) {
    FILE *writePtr = fopen("accounts.txt", "w");
    if (!writePtr) {
        perror("Text file error");
        return;
    }

    struct clientData client;
    rewind(readPtr);

    fprintf(writePtr, "%-6s%-16s%-11s%10s\n",
            "Acct", "Last Name", "First Name", "Balance");

    while (fread(&client, sizeof(client), 1, readPtr) == 1) {
        if (client.acctNum != 0) {
            fprintf(writePtr, "%-6u%-16s%-11s%10.2f\n",
                    client.acctNum,
                    client.lastName,
                    client.firstName,
                    client.balance);
        }
    }

    fclose(writePtr);
    printf("✔ accounts.txt generated successfully\n");
}

// ---------- ADD RECORD ----------
void newRecord(FILE *fPtr) {
    struct clientData client = {0};
    unsigned int account;

    if (!readUnsignedInt("Enter account number (1-100): ", &account)) {
        return;
    }

    if (!seekToRecord(fPtr, account)) return;

    if (fread(&client, sizeof(client), 1, fPtr) != 1) {
        printf("Failed to read account data\n");
        return;
    }

    if (client.acctNum != 0) {
        printf("Account already exists\n");
        return;
    }

    printf("Enter lastname firstname balance: ");
    if (scanf("%14s %9s %lf",
              client.lastName,
              client.firstName,
              &client.balance) != 3) {
        printf("Invalid input. Example: Kumar Ravi 1500.50\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    client.acctNum = account;

    fseek(fPtr, -sizeof(client), SEEK_CUR);
    if (fwrite(&client, sizeof(client), 1, fPtr) != 1) {
        printf("Failed to save account\n");
        return;
    }
    fflush(fPtr);

    printf("✔ Account created successfully\n");
}

// ---------- UPDATE ----------
void updateRecord(FILE *fPtr) {
    struct clientData client;
    unsigned int account;
    double transaction;

    if (!readUnsignedInt("Enter account: ", &account)) {
        return;
    }

    if (!seekToRecord(fPtr, account)) return;

    if (fread(&client, sizeof(client), 1, fPtr) != 1) {
        printf("Failed to read account data\n");
        return;
    }

    if (client.acctNum == 0) {
        printf("Account not found\n");
        return;
    }

    printf("Current balance: %.2f\n", client.balance);
    printf("Enter transaction (+/-): ");
    if (scanf("%lf", &transaction) != 1) {
        printf("Invalid transaction amount\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    client.balance += transaction;

    fseek(fPtr, -sizeof(client), SEEK_CUR);
    if (fwrite(&client, sizeof(client), 1, fPtr) != 1) {
        printf("Failed to update account\n");
        return;
    }
    fflush(fPtr);

    printf("✔ Updated balance: %.2f\n", client.balance);
}

// ---------- DELETE ----------
void deleteRecord(FILE *fPtr) {
    struct clientData client, blank = {0};
    unsigned int account;

    if (!readUnsignedInt("Enter account to delete: ", &account)) {
        return;
    }

    if (!seekToRecord(fPtr, account)) return;

    if (fread(&client, sizeof(client), 1, fPtr) != 1) {
        printf("Failed to read account data\n");
        return;
    }

    if (client.acctNum == 0) {
        printf("Account does not exist\n");
        return;
    }

    fseek(fPtr, -sizeof(client), SEEK_CUR);
    if (fwrite(&blank, sizeof(blank), 1, fPtr) != 1) {
        printf("Failed to delete account\n");
        return;
    }
    fflush(fPtr);

    printf("✔ Account deleted\n");
}

// ---------- LIST ----------
void listAccounts(FILE *fPtr) {
    struct clientData client;
    int count = 0;
    double totalBalance = 0.0;

    rewind(fPtr);
    printf("\n%-6s%-16s%-11s%10s\n",
           "Acct", "Last Name", "First Name", "Balance");

    while (fread(&client, sizeof(client), 1, fPtr) == 1) {
        if (client.acctNum != 0) {
            printf("%-6u%-16s%-11s%10.2f\n",
                   client.acctNum,
                   client.lastName,
                   client.firstName,
                   client.balance);
            count++;
            totalBalance += client.balance;
        }
    }

    printf("\nTotal active accounts: %d\n", count);
    printf("Total balance across all accounts: %.2f\n", totalBalance);
}

// ---------- MENU ----------
unsigned int enterChoice(void) {
    unsigned int choice;

    printf("\n1 - Generate text file\n"
           "2 - Update account\n"
           "3 - Add account\n"
           "4 - Delete account\n"
           "5 - List accounts\n"
           "6 - Exit\nChoice: ");

    if (scanf("%u", &choice) != 1) {
        printf("Invalid menu choice\n");
        clearInputBuffer();
        return 0;
    }
    clearInputBuffer();
    return choice;
}
