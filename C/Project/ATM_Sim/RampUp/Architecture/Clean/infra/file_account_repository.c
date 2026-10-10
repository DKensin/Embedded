
#include <stdio.h>
#include <string.h>

#include "account_repository.h"
#include "file_account_repository.h"

/* Example text format:
   account_id,balance
   ACC001,500000
*/

static bool file_find_by_id(
    void *context,
    const char *account_id,
    Account *out_account)
{
    FileAccountRepositoryContext *ctx = context;

    if (ctx == NULL || account_id == NULL ||
        out_account == NULL)
        return false;

    FILE *fp = fopen(ctx->file_path, "r");
    if (fp == NULL)
        return false;

    char line[128];
    bool found = false;

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        Account account;

        if (sscanf(line, "%31[^,],%lld",
                   account.account_id,
                   &account.balance) != 2)
            continue;

        if (strcmp(account.account_id, account_id) == 0)
        {
            *out_account = account;
            found = true;
            break;
        }
    }

    fclose(fp);
    return found;
}

/* Simplified placeholder: a production implementation
   must safely preserve all accounts and handle write errors. */
static bool file_save(
    void *context,
    const Account *account)
{
    (void)context;
    (void)account;

    /* Implement safe read-modify-write here. */
    return true;
}

AccountRepository file_account_repository_create(
    FileAccountRepositoryContext *context)
{
    AccountRepository repository = {
        .context = context,
        .find_by_id = file_find_by_id,
        .save = file_save
    };

    return repository;
}
