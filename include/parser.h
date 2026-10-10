#ifndef PARSER_H
#define PARSER_H

#include "table.h"

int parse_create_table(const char *input, Table *table);

ColumnType parse_column_type(const char *type);

int parse_insert(const char *input, char *table_name, char *values_text);

int parse_values(const char *values_text, char tokens[][MAX_NAME_LENGTH],
                 int *count);

int build_row(const Table *table, char tokens[][MAX_NAME_LENGTH],
              int token_count, Row *row);

int parse_select(const char *input, char *table_name);

int parse_select_column(const char *input, char *column_name, char *table_name);
int parse_select_columns(const char *input, char *columns_text,
                         char *table_name);

#endif
