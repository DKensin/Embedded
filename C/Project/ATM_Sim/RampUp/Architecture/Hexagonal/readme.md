1. Example explaination.
    - illustration Hexagonal design pattern
    - use withdraw use case, with data store in file (file adapter) and memory (memory adapter)
        + initial balance = 1.000.000 VND
        + user enter amount to withdraw money
        + reject in amound invalid or exceed the current balance
        + in case valid amound, update balance
        + can change the way to store data without change the withdraw logic
2. Include 3 components: Core, Port and Adapters
3. One Port can contains different Adapters (file, memory).