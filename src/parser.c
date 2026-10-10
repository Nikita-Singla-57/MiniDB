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
    const char *where_start = strstr(table_start, " WHERE ");

    size_t table_length;

    if (where_start != NULL) {
        table_length = (size_t)(where_start - table_start);
    }
    else {
        table_length = strlen(table_start);
    }

    while (table_length > 0 && table_start[table_length - 1] == ' ') {
        table_length--;
    }

    if (table_length == 0 || table_length >= MAX_NAME_LENGTH) {
        return 0;
    }

    strncpy(table_name, table_start, table_length);
    table_name[table_length] = '\0';

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
int parse_select_columns(const char *input, char *columns_text,
                         char *table_name) {
    const char *prefix = "SELECT ";
    const char *from_keyword = " FROM ";

    if (strncmp(input, prefix, strlen(prefix)) != 0) {
        return 0;
    }

    const char *columns_start = input + strlen(prefix);
    const char *from_start = strstr(columns_start, from_keyword);

    if (from_start == NULL) {
        return 0;
    }

    size_t columns_length = (size_t)(from_start - columns_start);

    if (columns_length == 0 || columns_length >= MAX_NAME_LENGTH) {
        return 0;
    }

    strncpy(columns_text, columns_start, columns_length);
    columns_text[columns_length] = '\0';

    const char *table_start = from_start + strlen(from_keyword);

    while (*table_start == ' ') {
        table_start++;
    }

    size_t table_length = strlen(table_start);

    while (table_length > 0 && table_start[table_length - 1] == ' ') {
        table_length--;
    }

    if (table_length == 0 || table_length >= MAX_NAME_LENGTH) {
        return 0;
    }

    strncpy(table_name, table_start, table_length);
    table_name[table_length] = '\0';

    return 1;
}

int parse_where_condition(const char *input, WhereCondition *condition) {
    const char *where_start = strstr(input, " WHERE ");

    if (where_start == NULL) {
        return 0;
    }

    const char *condition_start = where_start + strlen(" WHERE ");

    char column[MAX_NAME_LENGTH];
    char operator[3] = "";
    char value[MAX_NAME_LENGTH];

    int consumed = 0;

    if (sscanf(condition_start, "%49s %2s %49s%n", column, operator, value,
               &consumed) != 3) {
        return 0;
    }

    while (condition_start[consumed] == ' ') {
        consumed++;
    }

    if (condition_start[consumed] != '\0') {
        return 0;
    }

    if (strcmp(operator, "=") != 0 && strcmp(operator, "!=") != 0 &&
        strcmp(operator, ">") != 0 && strcmp(operator, "<") != 0 &&
        strcmp(operator, ">=") != 0 && strcmp(operator, "<=") != 0) {
        return 0;
    }

    strcpy(condition->column_name, column);
    strcpy(condition->operator, operator);
    strcpy(condition->value, value);

    return 1;
}
int parse_update(const char *input, char *table_name, char *column_name,
                 char *new_value) {
    const char *prefix = "UPDATE ";
    const char *set_text = " SET ";
    const char *where_text = " WHERE ";

    if (strncmp(input, prefix, strlen(prefix)) != 0) {
        return 0;
    }

    const char *table_start = input + strlen(prefix);
    const char *set_start = strstr(table_start, set_text);

    if (set_start == NULL) {
        return 0;
    }

    size_t table_length = (size_t)(set_start - table_start);

    if (table_length == 0 || table_length >= MAX_NAME_LENGTH) {
        return 0;
    }

    strncpy(table_name, table_start, table_length);
    table_name[table_length] = '\0';

    const char *assignment_start = set_start + strlen(set_text);
    const char *where_start = strstr(assignment_start, where_text);

    if (where_start == NULL) {
        return 0;
    }

    char assignment[MAX_NAME_LENGTH * 2];
    size_t assignment_length = (size_t)(where_start - assignment_start);

    if (assignment_length == 0 || assignment_length >= sizeof(assignment)) {
        return 0;
    }

    strncpy(assignment, assignment_start, assignment_length);
    assignment[assignment_length] = '\0';

    char *equals = strchr(assignment, '=');

    if (equals == NULL) {
        return 0;
    }

    *equals = '\0';

    if (strchr(equals + 1, '=') != NULL) {
        return 0;
    }

    char *column_start = assignment;
    while (*column_start == ' ') {
        column_start++;
    }

    char *column_end = column_start + strlen(column_start);
    while (column_end > column_start && column_end[-1] == ' ') {
        *--column_end = '\0';
    }

    char *value_start = equals + 1;
    while (*value_start == ' ') {
        value_start++;
    }

    char *value_end = value_start + strlen(value_start);
    while (value_end > value_start && value_end[-1] == ' ') {
        *--value_end = '\0';
    }

    if (*column_start == '\0' || *value_start == '\0' ||
        strlen(column_start) >= MAX_NAME_LENGTH ||
        strlen(value_start) >= MAX_NAME_LENGTH) {
        return 0;
    }

    strcpy(column_name, column_start);
    strcpy(new_value, value_start);

    return 1;
}

int parse_delete(const char *input, char *table_name) {
    const char *prefix = "DELETE FROM ";
    const char *where_text = " WHERE ";

    if (strncmp(input, prefix, strlen(prefix)) != 0) {
        return 0;
    }

    const char *table_start = input + strlen(prefix);
    const char *where_start = strstr(table_start, where_text);

    if (where_start == NULL) {
        return 0;
    }

    size_t table_length = (size_t)(where_start - table_start);

    if (table_length == 0 || table_length >= MAX_NAME_LENGTH) {
        return 0;
    }

    strncpy(table_name, table_start, table_length);
    table_name[table_length] = '\0';

    return 1;
}
int parse_drop_table(const char *input, char *table_name) {
    const char *prefix = "DROP TABLE ";

    if (strncmp(input, prefix, strlen(prefix)) != 0) {
        return 0;
    }

    const char *name_start = input + strlen(prefix);
    size_t name_length = strlen(name_start);

    if (name_length == 0 || name_length >= MAX_NAME_LENGTH) {
        return 0;
    }

    for (size_t i = 0; i < name_length; i++) {
        if (name_start[i] == ' ') {
            return 0;
        }
    }

    strcpy(table_name, name_start);
    return 1;
}

int parse_order_by(const char *input, char *column_name, int *descending) {
    const char *keyword = " ORDER BY ";
    const char *order_start = strstr(input, keyword);

    if (order_start == NULL) {
        return 0;
    }

    order_start += strlen(keyword);

    char column[MAX_NAME_LENGTH];
    char direction[10];
    int consumed = 0;

    if (sscanf(order_start, "%49s %9s%n", column, direction, &consumed) != 2) {
        return 0;
    }

    /* Remove a trailing semicolon from the direction. */
    size_t direction_length = strlen(direction);

    if (direction_length > 0 && direction[direction_length - 1] == ';') {
        direction[direction_length - 1] = '\0';
    }

    while (order_start[consumed] == ' ') {
        consumed++;
    }

    /* Also allow a semicolon after the direction. */
    if (order_start[consumed] == ';') {
        consumed++;
    }

    while (order_start[consumed] == ' ') {
        consumed++;
    }

    if (order_start[consumed] != '\0') {
        return 0;
    }

    if (strcmp(direction, "ASC") == 0) {
        *descending = 0;
    }
    else if (strcmp(direction, "DESC") == 0) {
        *descending = 1;
    }
    else {
        return 0;
    }

    strcpy(column_name, column);
    return 1;
}
