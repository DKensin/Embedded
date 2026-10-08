# ATM Software --- Architecture Design & Engineering Guide

> **Project:** ATM software simulation in C\
> **Initial storage:** Text files\
> **Architecture direction:** Clean Architecture + Hexagonal
> Architecture (Ports & Adapters) + Repository Pattern + State Machine\
> **Primary goals:** Modularity, maintainability, testability, and
> extensibility

------------------------------------------------------------------------

## 1. Project Overview

The project simulates the software of an ATM. Hardware is not required
initially; external devices are represented by software adapters.

### 1.1 Functional scope

-   Card insert and card validation
-   PIN authentication, retry limits, and lock/block policy
-   Cash withdrawal
-   Balance inquiry
-   Cash deposit
-   Account-to-account transfer
-   Change PIN
-   Transaction history
-   Receipt generation/printing
-   Cancel/eject card and cancel an operation where safe
-   Error handling and recovery
-   Text-file persistence initially

### 1.2 Non-functional goals

-   **Modularity:** components have clear responsibilities.
-   **Maintainability:** changes are localized.
-   **Testability:** business rules can be tested without a real
    database or device.
-   **Extensibility:** storage and device implementations can be
    replaced.
-   **Reliability:** errors are explicit; partial operations are handled
    deliberately.
-   **Security:** sensitive information is not stored or logged
    carelessly.
-   **Traceability:** operations and failures can be reviewed through
    transaction records and logs.

### 1.3 Scope and assumptions

1.  The initial version is a standalone, single-process ATM simulation.
2.  One interactive ATM session is active at a time; concurrent sessions
    are out of scope initially.
3.  The initial persistence mechanism is a set of text files, such as
    CSV or a documented line-based format.
4.  Hardware devices are simulated through interfaces/adapters.
5.  The application has a defined set of test accounts and cards for
    development.
6.  The first version focuses on core ATM workflows; networking,
    multi-ATM coordination, fraud analytics, multi-currency, and
    bank-grade integrations are future extensions.
7.  This is an educational software simulation, not production banking
    software.
8.  PIN handling in the simulation must still avoid plaintext PIN
    storage and PIN logging. A production design requires security
    review and appropriate cryptographic and hardware controls.

------------------------------------------------------------------------

## 2. Architecture at a Glance

The design combines several complementary ideas:

-   **Layered organization** groups components by responsibility.
-   **Clean Architecture** keeps source dependencies directed toward
    stable business rules.
-   **Hexagonal Architecture** exposes ports for communication with
    external systems and uses adapters to implement them.
-   **Repository Pattern** isolates data access behind repository
    contracts.
-   **State Machine** makes ATM session and transaction transitions
    explicit.

These are not competing choices. They address different aspects of the
same design.

### 2.1 High-level component diagram

``` mermaid
flowchart TB
    User[User]
    UI[Presentation / CLI<br/>Menus, input, display]
    APP[Application Layer<br/>Controllers and Use Cases]
    DOMAIN[Domain / Business Rules<br/>Account, Transaction, policies]
    PORTS[Ports / Interfaces<br/>Repository and device contracts]
    REPO[Repository Adapters<br/>Text-file storage initially]
    DEV[Device Adapters<br/>Simulated devices initially]
    FILES[(Text files<br/>accounts, cards, transactions, config)]
    HW[Simulated devices<br/>card reader, keypad, screen,<br/>cash dispenser, printer]

    User --> UI
    UI --> APP
    APP --> DOMAIN
    APP --> PORTS
    PORTS --> REPO
    PORTS --> DEV
    REPO --> FILES
    DEV --> HW
```

**Important dependency rule:** source code containing business rules
must not directly depend on concrete text-file functions, CLI functions,
or hardware-driver implementations. It should depend on stable
interfaces/contracts.

The diagram is conceptual: exact call paths depend on the use case. For
example, a use case may depend on an `AccountRepository` port and a
`CashDispenser` port.

### 2.2 Responsibility of each layer

  ----------------------------------------------------------------------------
  Layer / component Responsibility    Examples               Should avoid
  ----------------- ----------------- ---------------------- -----------------
  Presentation      Collect user      CLI menu, input        Business rules
                    input and show    parsing, display       and file parsing
                    results           messages               

  Application       Coordinate a user `WithdrawUseCase`,     Direct dependency
                    goal/use case     `TransferUseCase`      on concrete
                                                             storage/device
                                                             implementations

  Domain / Business Model concepts    Account rules, limits, UI, file formats,
                    and enforce       valid amount, transfer device drivers
                    business rules    invariants             

  Ports             Define contracts  `AccountRepository`,   Concrete storage
                    needed by core    `CashDispenser`        or hardware
                    logic                                    details

  Repository /      Implement         Text-file repository,  Owning core
  Device adapters   contracts         simulated printer      business policy

  Persistence /     Store records or  `.txt` files,          Being called
  external systems  simulate device   simulated dispenser    directly by
                    actions                                  domain rules
  ----------------------------------------------------------------------------

"Business Logic Layer" is sometimes used broadly to include both domain
rules and use-case orchestration. For this project, keep the distinction
useful: **Domain** owns core business rules; **Application** coordinates
the workflow.

------------------------------------------------------------------------

## 3. Why This Architecture?

### 3.1 Separation of concerns

Each component has a defined responsibility. For example, changing a
menu should not require changing withdrawal rules, and changing file
parsing should not require rewriting transfer validation.

### 3.2 Testability

Use cases can be tested with fake repositories and fake device ports.
Tests do not need to create real hardware or depend on the production
storage implementation.

### 3.3 Maintainability

Interfaces and consistent module boundaries make it easier to locate
defects and review changes.

### 3.4 Extensibility

The text-file repository can later be replaced by SQLite or another
database adapter. Simulated devices can later be replaced by real
drivers, provided the new implementation satisfies the same contract.

### 3.5 Controlled complexity

The project should not create an interface for every tiny function.
Introduce abstractions at boundaries where a dependency needs to be
isolated or where tests need substitution.

------------------------------------------------------------------------

## 4. Clean Architecture

Clean Architecture organizes software so that dependencies point toward
stable policy and business rules.

A practical mapping for this C project:

  -----------------------------------------------------------------------
  Concept                             ATM example
  ----------------------------------- -----------------------------------
  Entities / Domain model             `Account`, `Card`, `Transaction`,
                                      `Money`, policies

  Use Cases                           Withdraw, deposit, transfer,
                                      authenticate PIN

  Interface adapters                  Controllers, presenters, repository
                                      implementations, device adapters

  Frameworks and drivers              CLI implementation, file APIs,
                                      SQLite library, actual device SDKs
  -----------------------------------------------------------------------

The names do not require exactly four physical folders. The key is the
dependency direction.

### Dependency Rule

Inner business code must not include headers that expose concrete
infrastructure details.

Preferred:

``` text
Withdraw use case
    -> AccountRepository interface
        <- TextFileAccountRepository implementation
```

Avoid:

``` text
Withdraw use case
    -> fopen(), CSV parsing, "accounts.txt"
```

A source file may call an interface function pointer; a composition/root
module wires the interface to a concrete implementation at startup.

------------------------------------------------------------------------

## 5. Hexagonal Architecture: Ports and Adapters

Hexagonal Architecture treats the core as independent from outside
actors and systems.

### 5.1 Core

The core contains domain rules and application use cases.

### 5.2 Ports

A **port** is a contract through which the core communicates with
something outside it.

Examples:

-   `AccountRepository`: retrieve and save accounts
-   `TransactionRepository`: persist and query transaction records
-   `CardRepository`: validate/find card records, if needed
-   `CashDispenser`: dispense cash and report the result
-   `CardReader`: read/eject/retain a card
-   `Keypad`: receive PIN and menu input
-   `Display`: show prompts and results
-   `ReceiptPrinter`: print or simulate a receipt
-   `Clock`: supply time to make time-dependent tests deterministic

Ports can be inbound or outbound:

-   **Inbound port:** an application use case exposed to a caller,
    e.g. `withdraw_execute()`.
-   **Outbound port:** a dependency the core needs,
    e.g. `AccountRepository` or `CashDispenser`.

An inbound port does not have to use function pointers. A normal C
function is enough when there is one implementation and no runtime
dispatch requirement. Function pointers are useful when injecting
different implementations or test doubles.

### 5.3 Adapters

Adapters translate a port contract into a concrete implementation.

Examples:

-   `text_file_account_repository.c`
-   `sqlite_account_repository.c` (future)
-   `cash_dispenser_sim.c`
-   `cash_dispenser_hw.c` (future)
-   `cli_display.c`
-   `receipt_printer_sim.c`

Business logic should not care which adapter is selected.

------------------------------------------------------------------------

## 6. Repository Pattern

The Repository Pattern gives the application/domain a focused contract
for retrieving and persisting domain data without exposing the storage
mechanism.

### 6.1 Example contract in C

``` c
typedef struct {
    void *context;

    int (*get_by_id)(
        void *context,
        int account_id,
        Account *out_account
    );

    int (*save)(
        void *context,
        const Account *account
    );
} AccountRepository;
```

`context` allows an implementation to carry its own state, such as a
file path or database handle.

### 6.2 Example implementations

-   `TextFileAccountRepository`: reads/writes the initial text-file
    format.
-   `SqliteAccountRepository`: a possible future implementation.
-   `FakeAccountRepository`: an in-memory implementation for unit tests.

### 6.3 Repository design guidance

-   Use domain-level types in repository contracts, not raw CSV rows or
    SQL statements.
-   Return explicit status/error codes; do not silently ignore failed
    writes.
-   Document whether `save()` replaces a full record or updates selected
    fields.
-   Define ownership of returned data and buffers.
-   Keep file parsing and serialization inside the adapter.
-   Avoid one giant "DatabaseManager" that owns unrelated business
    logic.
-   Consider a transaction/unit-of-work abstraction when multiple
    records must change atomically.

### 6.4 Important consistency warning

A repository abstraction alone does **not** guarantee transactional
safety. A withdrawal may update an account and append a transaction
record; if one succeeds and the other fails, records can disagree.

For the text-file version, define a deliberate persistence strategy and
test failures. Options include a single canonical data file rewritten
through a temporary file plus atomic rename where supported, a
journal/recovery scheme, or using SQLite when transaction guarantees
become important. A production banking system needs stronger persistence
and security guarantees.

------------------------------------------------------------------------

## 7. ATM Device Abstraction Layer

Hardware is simulated initially, but it should still be isolated behind
contracts.

  -----------------------------------------------------------------------
  Device port             Typical operations      Simulation example
  ----------------------- ----------------------- -----------------------
  Card reader             Read, eject, retain     Prompt for a test card
                          card                    ID

  Keypad                  Read menu selection /   CLI input
                          PIN input               

  Display                 Show prompt/status      Terminal output

  Cash dispenser          Dispense amount and     Print a simulated
                          report outcome          dispensing message

  Receipt printer         Print receipt           Write receipt to
                                                  console or a file

  Deposit module          Accept and validate     Ask for a simulated
  (optional)              deposited cash          amount
  -----------------------------------------------------------------------

Device adapters should report failure explicitly. A message printed to
the console is not equivalent to a real device confirming that cash was
dispensed.

### Critical withdrawal distinction

A real-world withdrawal is a coordinated workflow involving account
posting and a physical cash outcome. Do not assume that a database
update and a dispenser action can be made atomic as if they were one
operation.

Model outcomes such as:

-   Rejected before posting
-   Posting succeeded and cash dispensed
-   Dispense failed before cash was delivered
-   Dispense outcome uncertain (for example, a communication failure
    after issuing the command)

For the simulation, define deterministic outcomes and recovery behavior.
For a real ATM, cash-reconciliation and reversal/compensation procedures
require dedicated design and domain expertise.

------------------------------------------------------------------------

## 8. ATM Session and Transaction State Machine

ATM behavior is stateful. A state machine helps prevent invalid
sequences, such as allowing withdrawal before authentication.

### 8.1 Example session states

``` mermaid
stateDiagram-v2
    [*] --> IDLE
    IDLE --> CARD_INSERTED: card presented
    CARD_INSERTED --> PIN_AUTH: card accepted
    CARD_INSERTED --> IDLE: invalid card / eject
    PIN_AUTH --> AUTHENTICATED: PIN accepted
    PIN_AUTH --> PIN_AUTH: wrong PIN, retries remain
    PIN_AUTH --> IDLE: retries exhausted / retain or eject policy
    AUTHENTICATED --> TRANSACTION: select operation
    TRANSACTION --> AUTHENTICATED: operation completed or safely cancelled
    TRANSACTION --> ERROR_RECOVERY: recoverable/system error
    ERROR_RECOVERY --> AUTHENTICATED: safe recovery
    ERROR_RECOVERY --> IDLE: terminate session
    AUTHENTICATED --> IDLE: cancel / eject
    IDLE --> [*]
```

This is a starting point, not a complete policy. Specify what happens
for every transition and failure.

### 8.2 Session state versus transaction state

Keep these concepts distinct:

-   **Session state:** card inserted, PIN authenticated, user menu, card
    eject/retain.
-   **Transaction state:** initiated, validated, pending, committed,
    failed, reversed/compensated, or outcome uncertain.

A user pressing "Cancel" should not blindly undo a committed operation.
Cancellation is only allowed while the operation is in a state where it
can be safely cancelled.

### 8.3 Suggested implementation

-   Use an enum for states.
-   Centralize allowed transitions in one module.
-   Reject unexpected events with explicit errors.
-   Test every valid transition and important invalid transition.
-   Avoid spreading state changes across unrelated controllers.

------------------------------------------------------------------------

## 9. Feature-to-Component Mapping

  ---------------------------------------------------------------------------------------------
  Feature           Application use case/controller  Domain/business      Ports/adapters
                                                     rules                commonly used
  ----------------- -------------------------------- -------------------- ---------------------
  Card insert       `CardInsertUseCase`              Card status and      Card reader, card
                                                     eligibility          repository

  PIN               `AuthenticatePinUseCase`         PIN verification,    Card/account
  authentication                                     retry/lock policy    repository, PIN
                                                                          verifier

  Withdraw          `WithdrawUseCase`                Amount, balance,     Account repository,
                                                     limits, transaction  transaction
                                                     policy               repository, cash
                                                                          dispenser

  Balance inquiry   `CheckBalanceUseCase`            Account access and   Account repository,
                                                     balance rules        display

  Deposit           `DepositUseCase`                 Valid amount and     Account repository,
                                                     posting rules        transaction
                                                                          repository, deposit
                                                                          module or simulator

  Transfer          `TransferUseCase`                Source/destination   Account and
                                                     validation, limits,  transaction
                                                     balanced posting     repositories

  Change PIN        `ChangePinUseCase`               Current PIN          Card/account
                                                     verification and new repository, PIN
                                                     PIN policy           verifier

  Transaction       `GetTransactionHistoryUseCase`   Authorization and    Transaction
  history                                            query rules          repository

  Receipt           `GenerateReceiptUseCase`         Receipt content and  Transaction/account
                                                     disclosure rules     data, receipt printer

  Cancel/eject      `CancelSessionUseCase` /         Safe cancellation    Card reader, session
                    `EjectCardUseCase`               policy and session   state
                                                     transition           

  Error handling    Shared result/error model and    Error classification Logging,
                    recovery workflows               and safe recovery    repositories, devices
                                                     policy               
  ---------------------------------------------------------------------------------------------

A controller is not automatically a business service. Controllers handle
incoming interaction; use cases coordinate a user goal; domain logic
enforces business rules.

------------------------------------------------------------------------

## 10. Error Handling Strategy

Use a consistent result/error model across modules.

Possible categories:

-   Input error: invalid amount or malformed input
-   Authentication error: invalid PIN, retries exhausted
-   Business rejection: insufficient funds, limit exceeded, inactive
    account
-   Not found: unknown account/card/transaction
-   Storage error: file missing, malformed record, write failure
-   Device error: dispenser/printer/card-reader failure
-   State error: operation not allowed in the current state
-   Internal error: unexpected invariant violation

Guidelines:

1.  Do not silently ignore I/O failures.
2.  Keep user-facing messages separate from internal diagnostic details.
3.  Never log PINs or secrets.
4.  Use explicit status codes or a consistent result structure.
5.  Define rollback/compensation behavior per workflow; not every error
    can be solved by simply restoring an in-memory value.
6.  Make error paths testable through fake adapters.
7.  Ensure error handling does not accidentally reveal whether sensitive
    credentials or accounts exist.

------------------------------------------------------------------------

## 11. Data Model and Text-File Persistence

Choose and document a stable file format before implementing
repositories. CSV is readable but requires correct escaping and
validation; a simple line-based format is acceptable if its grammar is
clearly defined.

Possible logical records:

### Account

-   `account_id`
-   `customer_id` (optional in the first version)
-   `balance` in the smallest currency unit (e.g. cents, not
    floating-point)
-   `status`
-   applicable limits or references to configuration

### Card

-   `card_id` or a non-sensitive test identifier
-   `account_id`
-   `status`
-   PIN verifier/hash metadata, if implemented

### Transaction

-   `transaction_id`
-   `type`
-   `source_account_id`
-   `destination_account_id` (optional)
-   `amount_minor_units`
-   `timestamp`
-   `status`
-   reference/reversal information where applicable

### Configuration

-   maximum PIN attempts
-   withdrawal limits
-   supported denominations
-   currency
-   receipt settings

Avoid storing plaintext PINs. For a learning simulation, use a suitable
password/PIN hashing library and document its limitations; do not invent
a custom cryptographic scheme. A real ATM uses a much more specialized
security architecture.

### Data integrity rules

-   Use integer minor units for money; avoid binary floating-point for
    balances.
-   Validate records when loading files.
-   Define behavior for duplicate IDs, missing files, corrupt rows, and
    partial writes.
-   Do not treat an in-memory update as persisted until the adapter
    confirms success.
-   Preserve transaction history for auditability in the simulation.
-   Restrict access to data files and avoid committing real credentials
    or personal data to source control.

------------------------------------------------------------------------

## 12. Suggested C Source Layout

One possible layout (adjust it to the actual size of the project):

``` text
atm/
├── CMakeLists.txt
├── README.md
├── docs/
│   ├── architecture.md
│   ├── requirements.md
│   └── test-plan.md
├── include/
│   ├── domain/
│   │   ├── account.h
│   │   ├── card.h
│   │   └── transaction.h
│   ├── application/
│   │   ├── withdraw_use_case.h
│   │   └── transfer_use_case.h
│   ├── ports/
│   │   ├── account_repository.h
│   │   ├── transaction_repository.h
│   │   └── cash_dispenser.h
│   └── common/
│       ├── result.h
│       └── error_code.h
├── src/
│   ├── domain/
│   ├── application/
│   ├── adapters/
│   │   ├── persistence/
│   │   │   └── text_file/
│   │   ├── devices/
│   │   │   └── simulated/
│   │   └── presentation/
│   │       └── cli/
│   └── bootstrap/
│       └── main.c
├── data/
│   ├── accounts.example.txt
│   ├── cards.example.txt
│   └── transactions.example.txt
└── tests/
    ├── unit/
    ├── integration/
    └── acceptance/
```

### Layout notes

-   Keep public interfaces in headers and implementation details in `.c`
    files.
-   Avoid cyclic `#include` dependencies.
-   Include the narrowest header required.
-   Use include guards.
-   `main.c` or a composition-root module should construct adapters and
    inject them into use cases.
-   Do not store real credentials or personal banking information in
    example data.
-   The folder layout is a guide, not a requirement to create empty
    modules prematurely.

------------------------------------------------------------------------

## 13. Testing Strategy

### Unit tests

Test domain rules and use cases without real file I/O or hardware.

Examples:

-   Withdraw rejects zero/negative amount.
-   Withdraw rejects insufficient funds.
-   Transfer rejects invalid destination.
-   PIN retry counter locks or terminates according to policy.
-   A cancel request is rejected after an irreversible commit point.

### Adapter tests

Test each concrete adapter independently:

-   Valid and malformed file rows
-   Missing/unwritable file
-   Duplicate IDs
-   Read/write failure
-   Simulated dispenser success and failure

### Integration tests

Test use cases with actual text-file adapters and simulated devices in a
temporary test data directory.

### End-to-end / acceptance tests

Test complete workflows from card insertion through authentication,
transaction selection, outcome display, receipt, and card ejection.

### Coverage and quality checks

-   Unit-test and integration-test suites
-   Branch coverage, not only line coverage
-   Compiler warnings treated seriously
-   Static analysis (for example, a suitable C static-analysis tool)
-   Sanitizers where supported by the compiler/toolchain
-   Regression tests on each pull request and a scheduled full
    regression run

Tests should not depend on production-like data or a developer's
personal file paths.

------------------------------------------------------------------------

## 14. Security and Reliability Checklist

-   [ ] No plaintext PIN storage or PIN logging
-   [ ] PIN attempts and lock/retention behavior are defined
-   [ ] Money uses integer minor units
-   [ ] Every repository operation checks its result
-   [ ] Malformed/corrupt files are handled safely
-   [ ] No partial transfer can silently debit one account without
    crediting the other
-   [ ] Withdrawal distinguishes account posting from physical
    dispensing outcome
-   [ ] Cancel is state-aware and cannot erase a committed transaction
-   [ ] Sensitive values are not included in error messages
-   [ ] Test data contains no real personal or financial information
-   [ ] Storage and device failure scenarios are tested
-   [ ] File replacement and recovery behavior are documented

------------------------------------------------------------------------

## 15. Implementation Roadmap

### Phase 1 --- Foundation

-   Define requirements, terminology, error codes, and domain types.
-   Define session states and transaction states.
-   Create build system and test framework.
-   Agree on text-file formats.

### Phase 2 --- Ports and adapters

-   Define repository contracts.
-   Implement text-file adapters.
-   Define device contracts.
-   Implement simulated card reader, keypad, display, cash dispenser,
    and printer.

### Phase 3 --- Core use cases

Implement and test in an incremental order:

1.  Card insert/eject and session state
2.  PIN authentication
3.  Balance inquiry
4.  Withdraw
5.  Deposit
6.  Transaction history and receipts
7.  Change PIN
8.  Transfer
9.  Cancellation and error recovery

### Phase 4 --- Quality

-   Add unit, adapter, integration, and acceptance tests.
-   Add branch-coverage reporting and static analysis.
-   Add negative/fault-injection tests.
-   Review dependencies and remove unnecessary coupling.

### Phase 5 --- Future extensions

-   Replace text-file storage with SQLite.
-   Add stronger transaction and crash-recovery handling.
-   Add real hardware adapters if appropriate.
-   Add network/bank integration only after defining authentication,
    authorization, protocol, security, and failure-recovery
    requirements.

------------------------------------------------------------------------

## 16. Key Decisions Summary

  -----------------------------------------------------------------------
  Decision                            Reason
  ----------------------------------- -----------------------------------
  Clean Architecture principles       Protect business rules from
                                      technology-specific changes

  Hexagonal Ports & Adapters          Make external dependencies
                                      replaceable and testable

  Repository Pattern                  Isolate retrieval and persistence
                                      from use cases

  State Machine                       Make valid ATM session transitions
                                      explicit

  Text-file adapter initially         Simple, inspectable storage for
                                      learning

  Simulated device adapters           Test workflows without physical ATM
                                      hardware

  Dependency injection at startup     Select real/fake implementations
                                      without coupling the core

  Explicit errors and recovery        Prevent silent failures and
  policies                            inconsistent state

  Incremental testing and CI          Detect regressions early and
                                      support maintainable development
  -----------------------------------------------------------------------

## 17. Final Design Principles

1.  **Keep business rules independent of the CLI, text files, and device
    implementations.**
2.  **Depend on interfaces at architectural boundaries; do not add
    abstractions without a clear reason.**
3.  **Treat a use case as a workflow, not just a wrapper around another
    function.**
4.  **Model session and transaction state explicitly.**
5.  **Design failure paths alongside success paths.**
6.  **Never assume file persistence and physical cash dispensing form
    one atomic operation.**
7.  **Keep the initial implementation small, but make important
    boundaries clear.**

> This architecture is a learning-oriented foundation. It is not, by
> itself, sufficient for deployment in a real banking ATM, which
> requires substantial additional security, hardware, operational,
> compliance, and transaction-recovery engineering.
