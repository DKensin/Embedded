#include <stdio.h>

#include "withdraw_controller.h"
#include "withdraw_presenter.h"
#include "file_account_repository.h"

int main(void)
{
    FileAccountRepositoryContext context = {
        .file_path = "data/accounts.txt"
    };

    AccountRepository repository = file_account_repository_create(&context);

    WithdrawRequest request = {
        .account_id = "ACC001",
        .amount = 10000
    };

    printf("ATM Withdraw demo\n");
    printf("Account: %s\n", request.account_id);
    printf("Requested withdrawal: %lld cents\n", request.amount);

    WithdrawResponse response = withdraw_controller_handle(
        &repository, request.account_id, request.amount);

    withdraw_presenter_show(&response);

    return 0;
}
