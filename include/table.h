#ifndef TABLE_H
#define TABLE_H

#define MAX_COLUMNS 10
#define MAX_NAME_LENGTH 50

typedef enum
{
    TYPE_INT,
    TYPE_STRING,
    TYPE_FLOAT
} ColumnType;

typedef struct
{
    char name[MAX_NAME_LENGTH];
    ColumnType type;
} Column;

typedef struct
{
    char name[MAX_NAME_LENGTH];
    Column columns[MAX_COLUMNS];
    int column_count;
} Table;

void init_table(Table *table, const char *name);

int add_column(Table *table, const char *name, ColumnType type);

void print_table(const Table *table);
#endif
