#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#define MAX 100
#define WIDTH 80

typedef struct {
    int id;
    char name[50], username[50], password[50], creationDate[50];
    float balance;
} Account;

typedef struct {
    char transactions[10][100];
    int top;
} Stack;

Account accounts[MAX];
Stack history[MAX];
int count = 0;

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Print blank line n times
void printBlankLines(int n) {
    for (int i=0; i<n; i++) printf("\n");
}

// Center and print text with optional vertical padding before.
// Adds padTop blank lines before printing, padBottom blank lines after printing
void printCenteredWithPadding(const char *text, int padTop, int padBottom) {
    printBlankLines(padTop);

    int len = (int)strlen(text);
    int pad = (WIDTH - len) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) printf(" ");
    printf("%s\n", text);

    printBlankLines(padBottom);
}

void pressEnter() {
    printBlankLines(1);
    const char *prompt = "Press Enter to continue...";
    int pad = (WIDTH - (int)strlen(prompt)) / 2;
    if (pad < 0) pad = 0;
    for (int i=0; i<pad; i++) printf(" ");
    printf("%s", prompt);
    while (getchar() != '\n');
}

void readLine(char *buffer, int size) {
    if (fgets(buffer, size, stdin)) {
        buffer[strcspn(buffer, "\n")] = 0;
    }
}

// Wrapper for input int with centered prompt
int inputInt(const char *prompt) {
    int val;
    int ok;
    char buffer[256];
    do {
        printBlankLines(1);
        int pad = (WIDTH - (int)strlen(prompt)) / 2;
        if (pad < 0) pad = 0;
        for (int i=0; i<pad; i++) printf(" ");
        printf("%s", prompt);

        ok = scanf("%d", &val);
        while (getchar() != '\n');
        if (!ok) {
            printCenteredWithPadding("Invalid input. Please enter a valid number.", 1, 1);
        } else break;
    } while(1);
    return val;
}

// Wrapper for input float with centered prompt
float inputFloat(const char *prompt, int nonNegative) {
    float val;
    int ok;
    do {
        printBlankLines(1);
        int pad = (WIDTH - (int)strlen(prompt)) / 2;
        if (pad < 0) pad = 0;
        for (int i=0; i<pad; i++) printf(" ");
        printf("%s", prompt);

        ok = scanf("%f", &val);
        while (getchar() != '\n');
        if (!ok || (nonNegative && val < 0)) {
            printCenteredWithPadding("Invalid input. Please enter a valid number.", 1, 1);
        } else break;
    } while(1);
    return val;
}

void maskPassword(char *password) {
#ifdef _WIN32
    char ch;
    int i = 0;
    while ((ch = _getch()) != '\r') {
        if (ch == '\b' && i > 0) {
            i--;
            printf("\b \b");
        }
        else if (i < 49) {
            password[i++] = ch;
            printf("*");
        }
    }
    password[i] = '\0';
    printf("\n");
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    int i = 0, ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        if ((ch == 127 || ch == 8) && i > 0) {
            i--;
            printf("\b \b");
        }
        else if (i < 49) {
            password[i++] = (char)ch;
            printf("*");
        }
    }
    password[i] = '\0';
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
#endif
}

void push(int index, const char *msg) {
    if (history[index].top < 9) {
        history[index].top++;
        strncpy(history[index].transactions[history[index].top], msg, 99);
        history[index].transactions[history[index].top][99] = '\0';
    } else {
        for (int i = 0; i < 9; i++)
            strcpy(history[index].transactions[i], history[index].transactions[i + 1]);
        strncpy(history[index].transactions[9], msg, 99);
        history[index].transactions[9][99] = '\0';
    }
}

void showHistory(int index) {
    printCenteredWithPadding("--- Transaction History (Oldest to Newest) ---", 1, 1);
    if (history[index].top == -1) {
        printCenteredWithPadding("No transactions yet.", 1, 1);
    } else {
        for (int i = 0; i <= history[index].top; i++) {
            char line[100];
            snprintf(line, sizeof(line), "%d. %s", i+1, history[index].transactions[i]);
            int pad = (WIDTH - (int)strlen(line)) / 2;
            if (pad < 0) pad = 0;
            for (int j=0; j < pad; j++) printf(" ");
            printf("%s\n", line);
        }
        printBlankLines(1);
    }
}

void loadData() {
    FILE *fp = fopen("accounts.txt", "r");
    if (!fp) return;
    while (count < MAX && fscanf(fp, "%d %[^\n] %[^\n] %[^\n] %f %[^\n]",
        &accounts[count].id,
        accounts[count].name,
        accounts[count].username,
        accounts[count].password,
        &accounts[count].balance,
        accounts[count].creationDate) == 6) {
        history[count].top = -1;
        count++;
    }
    fclose(fp);
}

void saveData() {
    FILE *fp = fopen("accounts.txt", "w");
    if (!fp) {
        printCenteredWithPadding("Error opening file for writing.", 1, 1);
        return;
    }
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d %s %s %s %.2f %s\n",
            accounts[i].id,
            accounts[i].name,
            accounts[i].username,
            accounts[i].password,
            accounts[i].balance,
            accounts[i].creationDate);
    }
    fclose(fp);
}

void createAccount() {
    if (count >= MAX) {
        printCenteredWithPadding("Account limit reached.", 1, 1);
        pressEnter();
        return;
    }
    clearScreen();
    printCenteredWithPadding("=== Create New Account ===", 2, 1);

    int new_id, exists;
    do {
        new_id = inputInt("Enter unique ID (integer): ");
        exists = 0;
        for (int i = 0; i < count; i++)
            if (accounts[i].id == new_id) {
                exists = 1;
                printCenteredWithPadding("ID already exists. Please enter a different ID.", 1,1);
                break;
            }
    } while (exists);
    accounts[count].id = new_id;

    printf("\n%*sEnter Full Name: ", (WIDTH - 14) / 2, "");
    readLine(accounts[count].name, sizeof(accounts[count].name));

    char username[50];
    do {
        printf("\n%*sChoose a Username: ", (WIDTH - 17) / 2, "");
        readLine(username, sizeof(username));
        exists = 0;
        for (int i = 0; i < count; i++) {
            if (strcmp(accounts[i].username, username) == 0) {
                exists = 1;
                printCenteredWithPadding("Username exists. Please choose another.", 1, 1);
                break;
            }
        }
    } while (exists);
    strcpy(accounts[count].username, username);

    printf("\n%*sChoose a Password: ", (WIDTH - 17) / 2, "");
    maskPassword(accounts[count].password);

    accounts[count].balance = inputFloat("Enter Initial Balance (>=0): ", 1);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    strftime(accounts[count].creationDate, sizeof(accounts[count].creationDate), "%d-%m-%Y %I:%M %p", &tm);

    history[count].top = -1;
    push(count, "Account created");
    count++;
    saveData();

    printCenteredWithPadding("Account created successfully!", 2, 2);
    pressEnter();
}

int login() {
    char user[50], pass[50];
    clearScreen();
    printCenteredWithPadding("=== Login ===", 2, 1);

    printf("\n%*sEnter Username: ", (WIDTH - 14) / 2, "");
    scanf("%49s", user); while (getchar() != '\n');

    printf("\n%*sEnter Password: ", (WIDTH - 14) / 2, "");
    maskPassword(pass);

    for (int i = 0; i < count; i++) {
        if (strcmp(accounts[i].username, user) == 0 &&
            strcmp(accounts[i].password, pass) == 0) return i;
    }
    return -1;
}

void deposit(int index) {
    clearScreen();
    printCenteredWithPadding("=== Deposit Money ===", 2, 1);

    float amt = inputFloat("Enter amount to deposit (>0): ", 0);
    if (amt <= 0) {
        printCenteredWithPadding("Amount must be > 0.", 2, 1);
        pressEnter();
        return;
    }
    accounts[index].balance += amt;

    char msg[100];
    snprintf(msg, sizeof(msg), "Deposited %.2f", amt);
    push(index, msg);
    saveData();

    printCenteredWithPadding("Deposit successful!", 2, 2);
    pressEnter();
}

void withdraw(int index) {
    clearScreen();
    printCenteredWithPadding("=== Withdraw Money ===", 2, 1);

    float amt = inputFloat("Enter amount to withdraw (>0): ", 0);
    if (amt <= 0) {
        printCenteredWithPadding("Amount must be > 0.", 2, 1);
        pressEnter();
        return;
    }
    if (accounts[index].balance < amt) {
        printCenteredWithPadding("Insufficient balance.", 2, 1);
        pressEnter();
        return;
    }
    accounts[index].balance -= amt;

    char msg[100];
    snprintf(msg, sizeof(msg), "Withdrew %.2f", amt);
    push(index, msg);
    saveData();

    printCenteredWithPadding("Withdrawal successful!", 2, 2);
    pressEnter();
}

void printAccountTableHeader() {
    printBlankLines(1);
    printf("%*s+--------+------------------------+------------------------+--------------+---------------------+\n", (WIDTH - 77) / 2, "");
    printf("%*s|   ID   | Name                   | Username               |   Balance    |     Created On       |\n", (WIDTH - 77) / 2, "");
    printf("%*s+--------+------------------------+------------------------+--------------+---------------------+\n", (WIDTH - 77) / 2, "");
}

void printAccountDetails(int index) {
    printf("%*s| %-6d | %-22s | %-22s | %12.2f | %-19s |\n",
        (WIDTH - 77) / 2, "",
        accounts[index].id,
        accounts[index].name,
        accounts[index].username,
        accounts[index].balance,
        accounts[index].creationDate);

    printf("%*s+--------+------------------------+------------------------+--------------+---------------------+\n\n", (WIDTH - 77) / 2, "");
}

void viewAccount(int index) {
    clearScreen();
    printCenteredWithPadding("=== Account Details ===", 2, 1);
    printAccountTableHeader();
    printAccountDetails(index);
    showHistory(index);
    pressEnter();
}

void updateAccountDetails(int index) {
    clearScreen();
    printCenteredWithPadding("=== Update Account Details ===", 2, 1);

    printCenteredWithPadding("1. Update Name", 1, 1);
    printCenteredWithPadding("2. Update Username", 0, 1);
    printCenteredWithPadding("3. Update Balance", 0, 2);

    printf("%*sChoose option: ", (WIDTH - 15) / 2, "");
    int option;
    if (scanf("%d", &option) != 1) {
        while (getchar() != '\n');
        printCenteredWithPadding("Invalid input.", 2, 2);
        pressEnter();
        return;
    }
    while (getchar() != '\n');

    switch (option) {
        case 1:
            printf("\n%*sEnter new name: ", (WIDTH - 14) / 2, "");
            readLine(accounts[index].name, sizeof(accounts[index].name));
            push(index, "Updated Name");
            break;
        case 2: {
            char username[50];
            int exists;
            do {
                printf("\n%*sEnter new username: ", (WIDTH - 17) / 2, "");
                readLine(username, sizeof(username));
                exists = 0;
                for (int i = 0; i < count; i++) {
                    if (i != index && strcmp(accounts[i].username, username) == 0) {
                        exists = 1;
                        printCenteredWithPadding("Username exists. Please choose another.", 2, 2);
                        break;
                    }
                }
            } while (exists);
            strcpy(accounts[index].username, username);
            push(index, "Updated Username");
            break;
        }
        case 3: {
            float new_balance = inputFloat("Enter new balance (>=0): ", 1);
            accounts[index].balance = new_balance;
            push(index, "Updated Balance");
            break;
        }
        default:
            printCenteredWithPadding("Invalid option.", 2, 2);
            pressEnter();
            return;
    }
    saveData();

    printCenteredWithPadding("Account updated successfully.", 2, 2);
    pressEnter();
}

void deleteAccount(int id) {
    int idx = -1;
    for (int i = 0; i < count; i++)
        if (accounts[i].id == id) {
            idx = i;
            break;
        }
    if (idx == -1) {
        printCenteredWithPadding("Account not found.", 2, 2);
        pressEnter();
        return;
    }
    for (int i = idx; i < count - 1; i++) {
        accounts[i] = accounts[i + 1];
        history[i] = history[i + 1];
    }
    count--;
    saveData();

    printCenteredWithPadding("Account deleted successfully.", 2, 2);
    pressEnter();
}

void transferFunds(int fromIndex) {
    int toId = inputInt("Enter recipient account ID: ");
    int toIndex = -1;
    for (int i = 0; i < count; i++)
        if (accounts[i].id == toId) {
            toIndex = i;
            break;
        }

    if (toIndex == -1) {
        printCenteredWithPadding("Recipient account not found.", 2, 2);
        pressEnter();
        return;
    }
    float amount = inputFloat("Enter amount to transfer (>0): ", 0);
    if (amount <= 0) {
        printCenteredWithPadding("Amount must be > 0.", 2, 2);
        pressEnter();
        return;
    }
    if (accounts[fromIndex].balance < amount) {
        printCenteredWithPadding("Insufficient balance.", 2, 2);
        pressEnter();
        return;
    }
    accounts[fromIndex].balance -= amount;
    accounts[toIndex].balance += amount;

    char msg[100];
    snprintf(msg, sizeof(msg), "Transferred %.2f to account %d", amount, toId);
    push(fromIndex, msg);

    snprintf(msg, sizeof(msg), "Received %.2f from account %d", amount, accounts[fromIndex].id);
    push(toIndex, msg);

    saveData();
    printCenteredWithPadding("Transfer successful!", 2, 2);
    pressEnter();
}

void exportTransactions(int index) {
    FILE *fp = fopen("transactions.txt", "w");
    if (!fp) {
        printCenteredWithPadding("Error opening file.", 2, 2);
        pressEnter();
        return;
    }
    for (int i = 0; i <= history[index].top; i++)
        fprintf(fp, "%s\n", history[index].transactions[i]);
    fclose(fp);

    printCenteredWithPadding("Transactions exported successfully to transactions.txt", 2, 2);
    pressEnter();
}

void printMiniStatement(int index) {
    clearScreen();
    printCenteredWithPadding("=== Mini Statement ===", 2, 2);
    int start = (history[index].top >= 4) ? history[index].top - 4 : 0;
    for (int i = start; i <= history[index].top; i++) {
        char line[100];
        snprintf(line, sizeof(line), "%d. %s", i - start + 1, history[index].transactions[i]);
        int pad = (WIDTH - (int)strlen(line)) / 2;
        if (pad < 0) pad = 0;
        for (int j=0; j < pad; j++) printf(" ");
        printf("%s\n", line);
    }
    printBlankLines(2);
    pressEnter();
}

void adminPanel() {
    clearScreen();
    printCenteredWithPadding("=== Admin Panel ===", 2, 2);
    printCenteredWithPadding("1. Search Account", 1, 1);
    printCenteredWithPadding("2. Delete Account", 0, 2);

    printf("%*sChoose option: ", (WIDTH - 15) / 2, "");
    int option;
    if (scanf("%d", &option) != 1) {
        while (getchar() != '\n');
        printCenteredWithPadding("Invalid input.", 2, 2);
        pressEnter();
        return;
    }
    while (getchar() != '\n');

    if (option == 1) {
        char keyword[50];
        printf("\n%*sEnter username to search: ", (WIDTH - 22) / 2, "");
        readLine(keyword, sizeof(keyword));

        int found = 0;
        clearScreen();
        for (int i = 0; i < count; i++) {
            if (strstr(accounts[i].username, keyword)) {
                printCenteredWithPadding("Account Found!", 2, 2);
                printAccountTableHeader();
                printAccountDetails(i);
                found = 1;
                break;
            }
        }
        if (!found) printCenteredWithPadding("No account matches the keyword.", 2, 2);
        pressEnter();
    } else if (option == 2) {
        int id = inputInt("Enter account ID to delete: ");
        deleteAccount(id);
    } else {
        printCenteredWithPadding("Invalid option.", 2, 2);
        pressEnter();
    }
}

int main() {
    loadData();

    while (1) {
        clearScreen();
        printCenteredWithPadding("=== Bank Management System ===", 2, 2);
        printCenteredWithPadding("1. Create Account", 1, 1);
        printCenteredWithPadding("2. Login", 0, 1);
        printCenteredWithPadding("3. Admin Panel", 0, 1);
        printCenteredWithPadding("4. Exit", 0, 2);

        printf("%*sChoose option: ", (WIDTH - 15) / 2, "");
        int choice;
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printCenteredWithPadding("Invalid input.", 2, 2);
            pressEnter();
            continue;
        }
        while (getchar() != '\n');

        if (choice == 1) {
            createAccount();
        } else if (choice == 2) {
            int loggedInIndex = login();
            if (loggedInIndex == -1) {
                printCenteredWithPadding("Login failed.", 2, 2);
                pressEnter();
                continue;
            }
            int userChoice;
            do {
                clearScreen();

                char userLine[100];
                snprintf(userLine, sizeof(userLine), "User: %s", accounts[loggedInIndex].name);
                printCenteredWithPadding(userLine, 2, 2);
                printCenteredWithPadding("--- User Menu ---", 2, 2);

                printCenteredWithPadding("1. View Account Details", 1, 1);
                printCenteredWithPadding("2. Deposit", 0, 1);
                printCenteredWithPadding("3. Withdraw", 0, 1);
                printCenteredWithPadding("4. Update Account", 0, 1);
                printCenteredWithPadding("5. Transfer Funds", 0, 1);
                printCenteredWithPadding("6. Export Transactions", 0, 1);
                printCenteredWithPadding("7. Mini Statement", 0, 1);
                printCenteredWithPadding("8. Show Transaction History", 0, 1);
                printCenteredWithPadding("9. Logout", 0, 2);

                printf("%*sChoose option: ", (WIDTH - 26) / 2, "");
                if (scanf("%d", &userChoice) != 1) {
                    while (getchar() != '\n');
                    printCenteredWithPadding("Invalid input.", 2, 2);
                    pressEnter();
                    continue;
                }
                while (getchar() != '\n');

                switch (userChoice) {
                    case 1: viewAccount(loggedInIndex); break;
                    case 2: deposit(loggedInIndex); break;
                    case 3: withdraw(loggedInIndex); break;
                    case 4: updateAccountDetails(loggedInIndex); break;
                    case 5: transferFunds(loggedInIndex); break;
                    case 6: exportTransactions(loggedInIndex); break;
                    case 7: printMiniStatement(loggedInIndex); break;
                    case 8:
                        clearScreen();
                        printCenteredWithPadding("--- Transaction History ---", 2, 2);
                        showHistory(loggedInIndex);
                        pressEnter();
                        break;
                    case 9: break;
                    default:
                        printCenteredWithPadding("Invalid option.", 2, 2);
                        pressEnter();
                }
            } while (userChoice != 9);
        } else if (choice == 3) {
            adminPanel();
        } else if (choice == 4) {
            saveData();
            printCenteredWithPadding("Exiting program...", 2, 2);
            break;
        } else {
            printCenteredWithPadding("Invalid option.", 2, 2);
            pressEnter();
        }
    }
    return 0;
}


