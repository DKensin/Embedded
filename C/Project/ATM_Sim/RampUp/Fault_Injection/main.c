
#include <stdio.h>
#include "fault_injection.h"

#define INITIAL_BALANCE 1000000
#define WITHDRAW_AMOUNT 200000

static int balance = INITIAL_BALANCE;

static int account_read(int *out_balance)
{
    if (fi_should_fail(FI_ACCOUNT_READ))
        return -1;

    *out_balance = balance;
    return 0;
}

static int balance_update(int new_balance)
{
    if (fi_should_fail(FI_BALANCE_UPDATE))
        return -1;

    balance = new_balance;
    return 0;
}

static int journal_write(void)
{
    if (fi_should_fail(FI_JOURNAL_WRITE))
        return -1;

    return 0;
}

static int withdraw(int amount)
{
    int current_balance;

    if (account_read(&current_balance) != 0)
    {
        printf("ERROR: Cannot read account\n");
        return -1;
    }

    if (amount <= 0 || current_balance < amount)
    {
        printf("ERROR: Invalid amount or insufficient funds\n");
        return -1;
    }

    if (balance_update(current_balance - amount) != 0)
    {
        printf("ERROR: Balance update failed\n");
        return -1;
    }

    if (journal_write() != 0)
    {
        printf("ERROR: Journal write failed\n");
        return -1;
    }

    printf("Withdraw successful: %d VND\n", amount);
    return 0;
}

static void run_test(const char *name, int expected_result)
{
    int result = withdraw(WITHDRAW_AMOUNT);

    if ((result == 0) == (expected_result == 0))
        printf("[PASS] %s\n", name);
    else
        printf("[FAIL] %s\n", name);
}

int main(void)
{
    printf("=== Test 1: Normal operation ===\n");

    fi_reset();
    balance = INITIAL_BALANCE;
    run_test("Normal withdrawal", 0);

    printf("\n=== Test 2: Account read failure ===\n");

    fi_reset();
    balance = INITIAL_BALANCE;
    fi_set(FI_ACCOUNT_READ, true);
    run_test("Account read failure", -1);

    printf("\n=== Test 3: Balance update failure ===\n");

    fi_reset();
    balance = INITIAL_BALANCE;
    fi_set(FI_BALANCE_UPDATE, true);
    run_test("Balance update failure", -1);

    printf("\n=== Test 4: Journal write failure ===\n");

    fi_reset();
    balance = INITIAL_BALANCE;
    fi_set(FI_JOURNAL_WRITE, true);
    run_test("Journal write failure", -1);

    printf("\n=== Test 5: Fail on second journal call ===\n");

    fi_reset();
    balance = INITIAL_BALANCE;
    fi_fail_on_nth_call(FI_JOURNAL_WRITE, 2);

    run_test("Journal call 1 succeeds", 0);
    run_test("Journal call 2 fails", -1);

    return 0;
}
