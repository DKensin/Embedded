#include <stdio.h>

int main(void)
{
    FILE *p;
    char *s = "Hello, World!";

    p = fopen("data.txt", "w");

    // printf("Enter account code:");
    // printf("Enter method: w/d");
    // printf("Enter amount: ");

    if (p == NULL)
    {
        printf("Error opening file.\n");
        return 1;
    }
    else
    {
        fwrite(s, sizeof(char), 13, p);
        fseek(p, -6, SEEK_END);
        fputc('!', p);
    }

    fclose(p);

    return 0;
}