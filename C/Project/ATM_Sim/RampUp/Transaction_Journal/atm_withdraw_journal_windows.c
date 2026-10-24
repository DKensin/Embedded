#define _POSIX_C_SOURCE 200809L

/*
 * ATM withdraw + transaction journal + recovery (educational example).
 * Build with Cygwin GCC: gcc -std=c11 -Wall -Wextra -Wpedantic atm_withdraw_journal_cygwin.c -o atm.exe
 * Run in Windows Command Prompt: atm.exe init 2000000
 *                                atm.exe withdraw TX1001 500000
 *                                atm.exe balance
 *                                atm.exe recover
 * Crash injection in Command Prompt: set ATM_CRASH_AT=after_commit
 *                                    atm.exe withdraw TX1002 100000
 *                                    set ATM_CRASH_AT=
 *                                    atm.exe recover
 *
 * Limitations: one account, one process/writer, no cash dispenser, no checksum,
 * simplified journal parser. Not production banking software. Use a DB for real use.
 */
/* Suppress selected MSVC warnings for standard C functions. */
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
/* Cygwin provides POSIX file-descriptor and synchronization APIs. */
#include <unistd.h>

/* Ask Cygwin to synchronize the file descriptor with the storage device. */
static int sync_fd(int fd)
{
    int result = -1;

    result = fsync(fd);

    return result;
}

/* Current account balance is stored separately from the append-only journal. */
#define ACCOUNT_FILE "account.txt"
#define ACCOUNT_TMP "account.tmp"
#define JOURNAL_FILE "transaction_journal.log"
#define MAX_ID 63
#define MAX_LINE 512

/* Journal line format: EVENT TX_ID ACCOUNT_ID BEFORE_BALANCE AFTER_BALANCE
 * Example: COMMIT TX1001 ACCOUNT_001 2000000 1500000 */

/* Transaction lifecycle recorded by journal events.
 * COMMITTED means the debit decision is durable; APPLIED means the balance file
 * has been updated and that completion has itself been journaled. */
typedef enum { NEW, BEGUN, PREPARED, COMMITTED, APPLIED, ABORTED, ERROR_STATE } TxState;
/* Snapshot of the transaction values needed to apply or recover a withdrawal.
 * before_balance and after_balance make recovery able to distinguish whether
 * the account file update happened before a crash. */
typedef struct {
    char id[MAX_ID + 1];
    char account[32];
    int64_t before_balance;
    int64_t after_balance;
    TxState state;
} Transaction;

/* Convert an internal enum to readable text for diagnostics. */
static const char *state_name(TxState s)
{
    const char *name = "ERROR";

    switch (s)
    {
        case NEW:
            name = "NEW";
            break;
        case BEGUN:
            name = "BEGUN";
            break;
        case PREPARED:
            name = "PREPARED";
            break;
        case COMMITTED:
            name = "COMMITTED";
            break;
        case APPLIED:
            name = "APPLIED";
            break;
        case ABORTED:
            name = "ABORTED";
            break;
        default:
            name = "ERROR";
            break;
    }

    return name;
}

/* Fault-injection hook: set ATM_CRASH_AT to simulate abrupt termination. */
static void crash_if(const char *point)
{
    const char *configured_point = getenv("ATM_CRASH_AT");

    if (configured_point != NULL && strcmp(configured_point, point) == 0)
    {
        fprintf(stderr, "*** Simulated crash at %s ***\n", point);
        fflush(stderr);
        _Exit(99); /* Intentionally bypass normal cleanup. */
    }
}

/* Flush stdio buffers, then request synchronization of the underlying file. */
static int durable_flush(FILE *fp)
{
    int result = -1;

    if (fflush(fp) == 0 && sync_fd(fileno(fp)) == 0)
    {
        result = 0;
    }

    return result;
}

/* Restrict transaction IDs to safe characters for the text journal. */
static int valid_id(const char *s)
{
    size_t length = strlen(s);
    size_t i;
    char c = '\0';
    int valid = 1;

    if (length == 0 || length > MAX_ID)
    {
        valid = 0;
    }

    for (i = 0; valid && i < length; ++i)
    {
        c = s[i];

        if (!((c >= 'a' && c <= 'z') ||
              (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') ||
              c == '_' || c == '-'))
        {
            valid = 0;
        }
    }

    return valid;
}

/* Append one event. Journal events are appended, never overwritten. */
static int append_record(const char *kind, const Transaction *tx)
{
    FILE *file = NULL;
    int result = -1;
    int write_ok = 0;

    file = fopen(JOURNAL_FILE, "ab");
    if (file == NULL)
    {
        perror("open journal");
    }
    else
    {
        write_ok = fprintf(file,
                           "%s %s %s %" PRId64 " %" PRId64 "\n",
                           kind,
                           tx->id,
                           tx->account,
                           tx->before_balance,
                           tx->after_balance) >= 0;

        if (!write_ok || durable_flush(file) != 0)
        {
            perror("write/flush journal");
        }
        else if (fclose(file) != 0)
        {
            file = NULL;
            perror("close journal");
        }
        else
        {
            file = NULL;
            result = 0;
        }

        if (file != NULL)
        {
            fclose(file);
        }
    }

    return result;
}

/* Load the single account balance from the account file. */
static int read_balance(int64_t *balance)
{
    FILE *file = NULL;
    int result = -1;

    file = fopen(ACCOUNT_FILE, "r");
    if (file != NULL)
    {
        if (fscanf(file, "%" SCNd64, balance) == 1)
        {
            result = 0;
        }

        if (fclose(file) != 0)
        {
            result = -1;
        }
    }

    return result;
}

/* Write the new balance to a temporary file, then replace the account file. */
static int write_balance_atomically(int64_t balance)
{
    FILE *file = NULL;
    int result = -1;
    int write_ok = 0;

    file = fopen(ACCOUNT_TMP, "wb");
    if (file == NULL)
    {
        perror("open account temp");
    }
    else
    {
        write_ok = fprintf(file, "%" PRId64 "\n", balance) >= 0;
        if (!write_ok || durable_flush(file) != 0)
        {
            perror("write/flush account temp");
        }
        else if (fclose(file) != 0)
        {
            file = NULL;
            perror("close account temp");
        }
        else
        {
            file = NULL;
            /*
             * Cygwin implements POSIX rename() semantics: when source and
             * destination are on the same filesystem, rename() replaces an
             * existing destination. The temporary file is in the same
             * directory as ACCOUNT_FILE to keep the replacement on one
             * filesystem.
             */
            if (rename(ACCOUNT_TMP, ACCOUNT_FILE) == 0)
            {
                result = 0;
            }
            else
            {
                perror("rename account temp");
            }
        }

        if (file != NULL)
        {
            fclose(file);
        }
    }

    return result;
}

/* Reconstruct a transaction by replaying its journal events in order. */
static int load_transaction(const char *wanted, Transaction *out, int *found)
{
    FILE *file = NULL;
    char line[MAX_LINE];
    char kind[24];
    char id[MAX_ID + 1];
    char account[32];
    char extra;
    int64_t before = 0;
    int64_t after = 0;
    int parse_count = 0;
    int io_error = 0;
    int result = 0;

    *found = 0;
    file = fopen(JOURNAL_FILE, "rb");
    if (file == NULL)
    {
        if (errno != ENOENT)
        {
            result = -1;
        }
    }
    else
    {
        while (result == 0 && fgets(line, sizeof line, file) != NULL)
        {
            parse_count = sscanf(line,
                                 "%23s %63s %31s %" SCNd64 " %" SCNd64 " %c",
                                 kind, id, account, &before, &after, &extra);

            if (parse_count != 5)
            {
                fprintf(stderr, "Malformed journal record; safe mode.\n");
                result = -1;
            }
            else if (strcmp(id, wanted) == 0)
            {
                if (!*found)
                {
                    memset(out, 0, sizeof *out);
                    snprintf(out->id, sizeof out->id, "%s", id);
                    snprintf(out->account, sizeof out->account, "%s", account);
                    out->before_balance = before;
                    out->after_balance = after;
                    out->state = NEW;
                    *found = 1;
                }
                else if (strcmp(out->account, account) != 0 ||
                         out->before_balance != before ||
                         out->after_balance != after)
                {
                    fprintf(stderr,
                            "Contradictory journal records for %s.\n",
                            wanted);
                    result = -1;
                }

                if (result == 0)
                {
                    if (strcmp(kind, "BEGIN") == 0 && out->state == NEW)
                    {
                        out->state = BEGUN;
                    }
                    else if (strcmp(kind, "PREPARED") == 0 && out->state == BEGUN)
                    {
                        out->state = PREPARED;
                    }
                    else if (strcmp(kind, "COMMIT") == 0 && out->state == PREPARED)
                    {
                        out->state = COMMITTED;
                    }
                    else if (strcmp(kind, "APPLIED") == 0 && out->state == COMMITTED)
                    {
                        out->state = APPLIED;
                    }
                    else if (strcmp(kind, "ABORT") == 0 &&
                             (out->state == BEGUN || out->state == PREPARED))
                    {
                        out->state = ABORTED;
                    }
                    else
                    {
                        fprintf(stderr,
                                "Illegal transition for %s at %s.\n",
                                wanted, kind);
                        result = -1;
                    }
                }
            }
        }

        io_error = ferror(file);
        if (io_error)
        {
            result = -1;
        }

        if (fclose(file) != 0)
        {
            result = -1;
        }
    }

    return result;
}

/* Recover interrupted transactions by rolling committed transactions forward. */
static int recover_all(void)
{
    FILE *file = NULL;
    char line[MAX_LINE];
    char ids[1024][MAX_ID + 1];
    char kind[24];
    char id[MAX_ID + 1];
    char account[32];
    char extra;
    int64_t before = 0;
    int64_t after = 0;
    int64_t current = 0;
    size_t id_count = 0;
    size_t i = 0;
    size_t j = 0;
    int parse_count = 0;
    int io_error = 0;
    int found = 0;
    int result = 0;
    Transaction tx;

    file = fopen(JOURNAL_FILE, "rb");
    if (file == NULL)
    {
        if (errno == ENOENT)
        {
            puts("No journal to recover.");
        }
        else
        {
            perror("open journal");
            result = -1;
        }
    }
    else
    {
        while (result == 0 && fgets(line, sizeof line, file) != NULL)
        {
            parse_count = sscanf(line,
                                 "%23s %63s %31s %" SCNd64 " %" SCNd64 " %c",
                                 kind, id, account, &before, &after, &extra);

            if (parse_count != 5)
            {
                fprintf(stderr,
                        "Malformed journal; safe mode, no recovery applied.\n");
                result = -1;
            }
            else
            {
                for (i = 0; i < id_count; ++i)
                {
                    if (strcmp(ids[i], id) == 0)
                    {
                        break;
                    }
                }

                if (i == id_count)
                {
                    if (id_count == 1024)
                    {
                        fprintf(stderr, "Demo recovery table full.\n");
                        result = -1;
                    }
                    else
                    {
                        snprintf(ids[id_count], sizeof ids[0], "%s", id);
                        ++id_count;
                    }
                }
            }
        }

        io_error = ferror(file);
        if (io_error)
        {
            result = -1;
        }
        if (fclose(file) != 0)
        {
            result = -1;
        }
    }

    for (j = 0; result == 0 && j < id_count; ++j)
    {
        found = 0;
        if (load_transaction(ids[j], &tx, &found) != 0 || !found)
        {
            result = -1;
        }
        else
        {
            switch (tx.state)
            {
                case BEGUN:
                case PREPARED:
                    /* No durable COMMIT means the balance must not be debited. */
                    if (append_record("ABORT", &tx) != 0)
                    {
                        result = -1;
                    }
                    else
                    {
                        printf("%s: %s -> ABORTED; balance unchanged.\n",
                               tx.id, state_name(tx.state));
                    }
                    break;

                case COMMITTED:
                    /* A committed transaction must be completed, not rolled back. */
                    if (read_balance(&current) != 0)
                    {
                        fprintf(stderr, "Cannot read account; safe mode.\n");
                        result = -1;
                    }
                    else if (current == tx.after_balance)
                    {
                        /* The account file is already updated; append APPLIED below. */
                    }
                    else if (current == tx.before_balance)
                    {
                        crash_if("recovery_before_account_replace");
                        if (write_balance_atomically(tx.after_balance) != 0)
                        {
                            result = -1;
                        }
                        else
                        {
                            crash_if("recovery_after_account_replace");
                        }
                    }
                    else
                    {
                        fprintf(stderr,
                                "%s: balance=%" PRId64 ", expected before=%" PRId64
                                " or after=%" PRId64 "; safe mode.\n",
                                tx.id, current, tx.before_balance, tx.after_balance);
                        result = -1;
                    }

                    if (result == 0)
                    {
                        if (append_record("APPLIED", &tx) != 0)
                        {
                            result = -1;
                        }
                        else
                        {
                            printf("%s: COMMITTED -> APPLIED; balance=%" PRId64 "\n",
                                   tx.id, tx.after_balance);
                        }
                    }
                    break;

                case APPLIED:
                    printf("%s: already APPLIED; no action.\n", tx.id);
                    break;

                case ABORTED:
                    printf("%s: already ABORTED; no action.\n", tx.id);
                    break;

                default:
                    fprintf(stderr, "%s: invalid state; safe mode.\n", tx.id);
                    result = -1;
                    break;
            }
        }
    }

    return result;
}

/* Initialize the demo account with a non-negative balance. */
static int command_init(const char *arg)
{
    char *end = NULL;
    long long value = 0;
    int result = 0;

    errno = 0;
    value = strtoll(arg, &end, 10);
    if (errno != 0 || end == NULL || *end != '\0' || value < 0)
    {
        fprintf(stderr, "Balance must be a non-negative integer.\n");
        result = 2;
    }
    else if (write_balance_atomically((int64_t)value) != 0)
    {
        result = 1;
    }
    else
    {
        printf("Initialized balance: %lld\n", value);
    }

    return result;
}

/* Print the current account balance without changing it. */
static int command_balance(void)
{
    int64_t balance = 0;
    int result = 0;

    if (read_balance(&balance) != 0)
    {
        fprintf(stderr, "Cannot read account.txt; run init first.\n");
        result = 1;
    }
    else
    {
        printf("Balance: %" PRId64 " VND (integer units for this demo)\n", balance);
    }

    return result;
}

/* Execute the withdrawal protocol from validation through durable completion. */
static int command_withdraw(const char *id, const char *arg)
{
    char *end = NULL;
    long long amount = 0;
    Transaction old_tx;
    Transaction tx;
    int found = 0;
    int64_t balance = 0;
    int result = 0;

    errno = 0;
    amount = strtoll(arg, &end, 10);
    if (errno != 0 || end == NULL || *end != '\0' || amount <= 0 || !valid_id(id))
    {
        fprintf(stderr, "Invalid transaction ID or amount.\n");
        result = 2;
    }
    else if (recover_all() != 0)
    {
        fprintf(stderr, "Recovery failed; refusing withdrawal.\n");
        result = 1;
    }
    else if (load_transaction(id, &old_tx, &found) != 0)
    {
        result = 1;
    }
    else if (found)
    {
        printf("Duplicate transaction ID %s (state=%s); refusing replay.\n",
               id, state_name(old_tx.state));
        result = 1;
    }
    else if (read_balance(&balance) != 0)
    {
        fprintf(stderr, "Cannot read account; run init first.\n");
        result = 1;
    }
    else if ((int64_t)amount > balance)
    {
        fprintf(stderr, "Insufficient funds.\n");
        result = 1;
    }
    else
    {
        /* Store before/after balances so recovery can determine whether the
         * account update happened before a crash. */
        memset(&tx, 0, sizeof tx);
        snprintf(tx.id, sizeof tx.id, "%s", id);
        snprintf(tx.account, sizeof tx.account, "ACCOUNT_001");
        tx.before_balance = balance;
        tx.after_balance = balance - (int64_t)amount;
        tx.state = NEW;

        crash_if("before_begin");
        if (append_record("BEGIN", &tx) != 0)
        {
            result = 1;
        }
        else
        {
            tx.state = BEGUN;
            crash_if("after_begin");

            if (append_record("PREPARED", &tx) != 0)
            {
                result = 1;
            }
            else
            {
                tx.state = PREPARED;
                crash_if("after_prepared");

                /* COMMIT is the decision point: recovery must roll forward. */
                if (append_record("COMMIT", &tx) != 0)
                {
                    fprintf(stderr,
                            "Commit result uncertain; run recovery, do not blindly retry.\n");
                    result = 1;
                }
                else
                {
                    tx.state = COMMITTED;
                    crash_if("after_commit");

                    if (write_balance_atomically(tx.after_balance) != 0)
                    {
                        fprintf(stderr,
                                "Committed; account update pending recovery.\n");
                        result = 1;
                    }
                    else
                    {
                        crash_if("after_account_replace");
                        if (append_record("APPLIED", &tx) != 0)
                        {
                            fprintf(stderr,
                                    "Account updated; APPLIED pending recovery.\n");
                            result = 1;
                        }
                        else
                        {
                            tx.state = APPLIED;
                            crash_if("after_applied");
                            printf("Withdrawal successful: tx=%s amount=%lld new_balance=%" PRId64 "\n",
                                   id, amount, tx.after_balance);
                        }
                    }
                }
            }
        }
    }

    return result;
}

/* Print supported commands and fault-injection point names. */
static void usage(const char *program_name)
{
    printf("Usage:\n"
           "  %s init <balance>\n"
           "  %s withdraw <tx_id> <amount>\n"
           "  %s balance\n"
           "  %s recover\n",
           program_name, program_name, program_name, program_name);
    puts("ATM_CRASH_AT points: before_begin, after_begin, after_prepared, after_commit,");
    puts("after_account_replace, after_applied, recovery_before_account_replace, recovery_after_account_replace");
}

/* Program entry point: dispatch command-line arguments to a command handler. */
int main(int argc, char *argv[])
{
    int result = 2;

    if (argc == 3 && strcmp(argv[1], "init") == 0)
    {
        result = command_init(argv[2]);
    }
    else if (argc == 4 && strcmp(argv[1], "withdraw") == 0)
    {
        result = command_withdraw(argv[2], argv[3]);
    }
    else if (argc == 2 && strcmp(argv[1], "balance") == 0)
    {
        result = command_balance();
    }
    else if (argc == 2 && strcmp(argv[1], "recover") == 0)
    {
        if (recover_all() != 0)
        {
            fprintf(stderr, "Recovery failed; ATM remains in safe mode.\n");
            result = 1;
        }
        else
        {
            result = command_balance();
        }
    }
    else
    {
        usage(argv[0]);
    }

    return result;
}
