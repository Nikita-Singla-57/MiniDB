#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

int parse_create_table(const char *input, Table *table) {
    const char *prefix = "CREATE TABLE ";

    if (strncmp(input, prefix, strlen(prefix)) != 0) {
        return 0;
    }

    const char *table_name_start = input + strlen(prefix);

    char table_name[MAX_NAME_LENGTH];

    int i = 0;

    while (table_name_start[i] != '\0' && table_name_start[i] != '(' &&
           !isspace((unsigned char)table_name_start[i]) &&
           i < MAX_NAME_LENGTH - 1) {
        table_name[i] = table_name_start[i];
        i++;
    }

    table_name[i] = '\0';

    if (i == 0) {
        return 0;
    }

    init_table(table, table_name);

    const char *columns_start = strchr(input, '(');
    const char *columns_end = strrchr(input, ')');

    if (columns_start == NULL || columns_end == NULL ||
        columns_end <= columns_start) {
        return 0;
    }

    char columns_text[500];

    size_t columns_length = columns_end - columns_start - 1;

    if (columns_length >= sizeof(columns_text)) {
        return 0;
    }

    strncpy(columns_text, columns_start + 1, columns_length);
    columns_text[columns_length] = '\0';

    char *column_text = strtok(columns_text, ",");

    while (column_text != NULL) {
        char column_name[MAX_NAME_LENGTH];
        char column_type[20];

        if (sscanf(column_text, " %49s %19s", column_name, column_type) != 2) {
            return 0;
        }

        ColumnType type = parse_column_type(column_type);

        if (type == (ColumnType)-1) {
            return 0;
        }

        if (!add_column(table, column_name, type)) {
            return 0;
        }

        column_text = strtok(NULL, ",");
    }

    return 1;
}

ColumnType parse_column_type(const char *type) {
    if (strcmp(type, "INT") == 0) {
        return TYPE_INT;
    }

    if (strcmp(type, "STRING") == 0) {
        return TYPE_STRING;
    }

    if (strcmp(type, "FLOAT") == 0) {
        return TYPE_FLOAT;
    }

    return -1;
}

int parse_insert(const char *input, char *table_name, char *values_text) {
    const char *prefix = "INSERT INTO ";

    if (strncmp(input, prefix, strlen(prefix)) != 0) {
        return 0;
    }

    const char *table_start = input + strlen(prefix);

    const char *values_keyword = strstr(table_start, " VALUES ");

    if (values_keyword == NULL) {
        return 0;
    }

    int table_length = values_keyword - table_start;

    if (table_length <= 0 || table_length >= MAX_NAME_LENGTH) {
        return 0;
    }

    strncpy(table_name, table_start, table_length);
    table_name[table_length] = '\0';

    const char *values_start = values_keyword + strlen(" VALUES ");

    if (values_start[0] != '(') {
        return 0;
    }

    const char *values_end = strrchr(values_start, ')');

    if (values_end == NULL || values_end <= values_start) {
        return 0;
    }

    int values_length = values_end - values_start - 1;

    if (values_length <= 0 || values_length >= 500) {
        return 0;
    }

    strncpy(values_text, values_start + 1, values_length);
    values_text[values_length] = '\0';

    return 1;
}

int parse_values(const char *values_text, char tokens[][MAX_NAME_LENGTH],
                 int *count) {
    int token_index = 0;
    int char_index = 0;
    int in_quotes = 0;

    *count = 0;

    for (int i = 0;; i++) {
        char current = values_text[i];

        if (current == '"') {
            in_quotes = !in_quotes;
            continue;
        }

        if ((current == ',' && !in_quotes) || current == '\0') {
            tokens[token_index][char_index] = '\0';

            token_index++;
            char_index = 0;

            if (current == '\0') {
                break;
            }

            if (token_index >= MAX_COLUMNS) {
                return 0;
            }

            continue;
        }

        if (char_index < MAX_NAME_LENGTH - 1) {
            tokens[token_index][char_index] = current;
            char_index++;
        }
        else {
            return 0;
        }
    }

    *count = token_index;

    return 1;
}

int build_row(const Table *table, char tokens[][MAX_NAME_LENGTH],
              int token_count, Row *row) {
    if (token_count != table->column_count) {
        return 0;
    }

    for (int i = 0; i < table->column_count; i++) {
        ColumnType type = table->columns[i].type;

        row->values[i].type = type;

        if (type == TYPE_INT) {
            char *end;
            long value = strtol(tokens[i], &end, 10);

            if (*end != '\0') {
                return 0;
            }

            row->values[i].data.int_value = (int)value;
        }
        else if (type == TYPE_FLOAT) {
            char *end;
            float value = strtof(tokens[i], &end);

            if (*end != '\0') {
                return 0;
            }

            row->values[i].data.float_value = value;
        }
        else if (type == TYPE_STRING) {
            strncpy(row->values[i].data.string_value, tokens[i],
                    MAX_NAME_LENGTH - 1);

            row->values[i].data.string_value[MAX_NAME_LENGTH - 1] = '\0';
        }
        else {
            return 0;
        }
    }

    return 1;
}

int parse_select(const char *input, char *table_name) {
    const char *prefix = "SELECT * FROM ";

    if (strncmp(input, prefix, strlen(prefix)) != 0) {
        return 0;
    }

    const char *table_start = input + strlen(prefix);

    if (*table_start == '\0') {
        return 0;
    }

    int length = strlen(table_start);

    if (length >= MAX_NAME_LENGTH) {
        return 0;
    }

    strcpy(table_name, table_start);

    return 1;
}

int parse_select_column(const char *input, char *column_name,
                        char *table_name) {
    const char *prefix = "SELECT ";
    const char *from_keyword = " FROM ";

    if (strncmp(input, prefix, strlen(prefix)) != 0) {
        return 0;
    }

    const char *column_start = input + strlen(prefix);
    const char *from_start = strstr(column_start, from_keyword);

    if (from_start == NULL) {
        return 0;
    }

    int column_length = (int)(from_start - column_start);

    if (column_length <= 0 || column_length >= MAX_NAME_LENGTH) {
        return 0;
    }

    strncpy(column_name, column_start, (size_t)column_length);
    column_name[column_length] = '\0';

    const char *table_start = from_start + strlen(from_keyword);

    while (*table_start == ' ') {
        table_start++;
    }

    int table_length = (int)strlen(table_start);

    while (table_length > 0 && table_start[table_length - 1] == ' ') {
        table_length--;
    }

    if (table_length <= 0 || table_length >= MAX_NAME_LENGTH) {
        return 0;
    }

    strncpy(table_name, table_start, (size_t)table_length);
    table_name[table_length] = '\0';

    return 1;
}
