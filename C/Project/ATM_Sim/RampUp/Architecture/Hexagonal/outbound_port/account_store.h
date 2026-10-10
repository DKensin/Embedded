#ifndef ACCOUNT_STORE_H
#define ACCOUNT_STORE_H

/* Outbound port will be used by Business Logic to access the balance */
typedef struct
{
    int (*load_balance)(int *balance);  /* request to load the balance */
    int (*save_balance)(int balance);   /* request to save the new balance */
} AccountStore;

#endif /* ACCOUNT_STORE_H */
