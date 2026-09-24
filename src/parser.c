#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "parser.h"

int parse_create_table(const char *input, Table *table)
{
    const char *prefix = "CREATE TABLE ";

    if (strncmp(input, prefix, strlen(prefix)) != 0)
    {
        return 0;
    }

    const char *table_name_start = input + strlen(prefix);

    char table_name[MAX_NAME_LENGTH];

    int i = 0;

    while (table_name_start[i] != '\0' &&
           table_name_start[i] != '(' &&
           !isspace((unsigned char)table_name_start[i]) &&
           i < MAX_NAME_LENGTH - 1)
    {
        table_name[i] = table_name_start[i];
        i++;
    }

    table_name[i] = '\0';

    if (i == 0)
    {
        return 0;
    }

    init_table(table, table_name);

    const char *columns_start = strchr(input, '(');
    const char *columns_end = strrchr(input, ')');

    if (columns_start == NULL || columns_end == NULL ||
        columns_end <= columns_start)
    {
        return 0;
    }

    char columns_text[500];

    size_t columns_length = columns_end - columns_start - 1;

    if (columns_length >= sizeof(columns_text))
    {
        return 0;
    }

    strncpy(columns_text, columns_start + 1, columns_length);
    columns_text[columns_length] = '\0';

    char *column_text = strtok(columns_text, ",");

    while (column_text != NULL)
    {
        char column_name[MAX_NAME_LENGTH];
        char column_type[20];

        if (sscanf(column_text, " %49s %19s",
                   column_name, column_type) != 2)
        {
            return 0;
        }

        ColumnType type = parse_column_type(column_type);

        if (type == (ColumnType)-1)
        {
            return 0;
        }

        if (!add_column(table, column_name, type))
        {
            return 0;
        }

        column_text = strtok(NULL, ",");
    }

    return 1;
}

ColumnType parse_column_type(const char *type)
{
    if (strcmp(type, "INT") == 0)
    {
        return TYPE_INT;
    }

    if (strcmp(type, "STRING") == 0)
    {
        return TYPE_STRING;
    }

    if (strcmp(type, "FLOAT") == 0)
    {
        return TYPE_FLOAT;
    }

    return -1;
}
