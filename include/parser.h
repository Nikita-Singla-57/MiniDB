#ifndef PARSER_H
#define PARSER_H

#include "table.h"

int parse_create_table(const char *input, Table *table);

ColumnType parse_column_type(const char *type);

#endif
