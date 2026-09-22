#include <stdio.h>
#include <string.h>

#include "cli.h"

void start_cli(void)
{
    char input[100];

    printf("Welcome to MiniDB!\n");
    printf("Type \".help\" for help.\n\n");

    while (1)
    {
        printf("MiniDB> ");

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, ".exit") == 0)
        {
            printf("Goodbye!\n");
            break;
        }
        else if (strcmp(input, ".help") == 0)
        {
            printf("Available commands:\n");
            printf(".help     Show this help message\n");
            printf(".version  Show MiniDB version\n");
            printf(".exit     Exit MiniDB\n");
        }
        else if (strcmp(input, ".version") == 0)
        {
            printf("MiniDB version 1.0\n");
        }
        else if (strlen(input) == 0)
        {
            continue;
        }
        else
        {
            printf("Unknown command: %s\n", input);
        }
    }
}
