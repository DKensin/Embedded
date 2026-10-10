
#ifndef FILE_ACCOUNT_REPOSITORY_H
#define FILE_ACCOUNT_REPOSITORY_H

typedef struct
{
    const char *file_path;
} FileAccountRepositoryContext;

AccountRepository file_account_repository_create(
    FileAccountRepositoryContext *context);

#endif /* FILE_ACCOUNT_REPOSITORY_H */
