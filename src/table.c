#include <stdio.h>
#include <string.h>

#include "table.h"

void init_table(Table *table, const char *name)
{
    strncpy(table->name, name, MAX_NAME_LENGTH - 1);
    table->name[MAX_NAME_LENGTH - 1] = '\0';

    table->column_count = 0;
    table->row_count = 0;
}

Table *find_table(Table tables[], int table_count, const char *name)
{
    for (int i = 0; i < table_count; i++)
    {
        if (strcmp(tables[i].name, name) == 0)
        {
            return &tables[i];
        }
    }
    return NULL;
}

int add_column(Table *table, const char *name, ColumnType type)
{
    if (table->column_count >= MAX_COLUMNS)
    {
        return 0;
    }

    Column *column = &table->columns[table->column_count];

    strncpy(column->name, name, MAX_NAME_LENGTH - 1);
    column->name[MAX_NAME_LENGTH - 1] = '\0';

    column->type = type;

    table->column_count++;

    return 1;
}

int add_row(Table *table, const Row *row)
{
    if (table->row_count >= MAX_ROWS)
    {
        return 0;
    }

    table->rows[table->row_count] = *row;
    table->row_count++;

    return 1;
}

void print_table(const Table *table)
{
    printf("Table: %s\n\n", table->name);
    printf("Columns:\n");

    for (int i = 0; i < table->column_count; i++)
    {
        printf("%d. %s ", i + 1, table->columns[i].name);

        switch (table->columns[i].type)
        {
            case TYPE_INT:
                printf("INT");
                break;

            case TYPE_STRING:
                printf("STRING");
                break;

            case TYPE_FLOAT:
                printf("FLOAT");
                break;
        }

        printf("\n");
    }
}
