#ifndef TABLE_H
#define TABLE_H

#define MAX_COLUMNS 10
#define MAX_NAME_LENGTH 50
#define MAX_TABLES 10
#define MAX_ROWS 100

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

typedef union
{
    int int_value;
    float float_value;
    char string_value[MAX_NAME_LENGTH];
} ValueData;

typedef struct
{
    ColumnType type;
    ValueData data;
} Value;

typedef struct
{
    Value values[MAX_COLUMNS];
} Row;

typedef struct
{
    char name[MAX_NAME_LENGTH];

    Column columns[MAX_COLUMNS];
    int column_count;

    Row rows[MAX_ROWS];
    int row_count;

} Table;

void init_table(Table *table, const char *name);

int add_column(Table *table, const char *name, ColumnType type);

int add_row(Table *table, const Row *row);

Table *find_table(Table tables[], int table_count, const char *name);

void print_row(const Table *table, const Row *row);

void print_table_rows(const Table *table);

void print_table(const Table *table);

#endif
