1. How to build:
    gcc -std=c99 -Wall -Wextra -Wpedantic atm_withdraw_journal_windows.c -o atm.exe

2. run
    .\atm.exe init 2000000
    .\atm.exe withdraw TX1001 500000
    .\atm.exe balance
    .\atm.exe recover

3. simulate crash after commit
    set ATM_CRASH_AT=after_commit
    .\atm.exe withdraw TX1002 100000
    set ATM_CRASH_AT=
    .\atm.exe recover