#ifndef FAULT_INJECTION_H
#define FAULT_INJECTION_H

#include <stdbool.h>

typedef enum
{
    FI_ACCOUNT_READ = 0,
    FI_BALANCE_UPDATE,
    FI_JOURNAL_WRITE,
    FI_RECEIPT_WRITE,
    FI_COUNT
} FaultId;

/* Enable or disable a fault point */
bool fi_set(FaultId id, bool enable);

/* Fail on the Nth call. N must be >= 1 */
bool fi_fail_on_nth_call(FaultId id, unsigned int n);

/* Check whether the current operation should fail */
bool fi_should_fail(FaultId id);

/* Reset all fault configurations and call counters */
void fi_reset(void);

#endif /* FAULT_INJECTION_H */
