#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "condition.h"
#include "table.h"

void init_table(Table *table, const char *name) {
    strncpy(table->name, name, MAX_NAME_LENGTH - 1);
    table->name[MAX_NAME_LENGTH - 1] = '\0';

    table->column_count = 0;
    table->row_count = 0;
}

Table *find_table(Table tables[], int table_count, const char *name) {
    for (int i = 0; i < table_count; i++) {
        if (strcmp(tables[i].name, name) == 0) {
            return &tables[i];
        }
    }
    return NULL;
}

int add_column(Table *table, const char *name, ColumnType type) {
    if (table->column_count >= MAX_COLUMNS) {
        return 0;
    }

    Column *column = &table->columns[table->column_count];

    strncpy(column->name, name, MAX_NAME_LENGTH - 1);
    column->name[MAX_NAME_LENGTH - 1] = '\0';

    column->type = type;

    table->column_count++;

    return 1;
}

int add_row(Table *table, const Row *row) {
    if (table->row_count >= MAX_ROWS) {
        return 0;
    }

    table->rows[table->row_count] = *row;
    table->row_count++;

    return 1;
}

void print_table(const Table *table) {
    printf("Table: %s\n\n", table->name);
    printf("Columns:\n");

    for (int i = 0; i < table->column_count; i++) {
        printf("%d. %s ", i + 1, table->columns[i].name);

        switch (table->columns[i].type) {
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

void print_row(const Table *table, const Row *row) {
    for (int i = 0; i < table->column_count; i++) {
        if (i > 0) {
            printf(" | ");
        }

        if (row->values[i].type == TYPE_INT) {
            printf("%d", row->values[i].data.int_value);
        }
        else if (row->values[i].type == TYPE_STRING) {
            printf("%s", row->values[i].data.string_value);
        }
        else if (row->values[i].type == TYPE_FLOAT) {
            printf("%.2f", row->values[i].data.float_value);
        }
    }

    printf("\n");
}

void print_table_rows(const Table *table) {
    for (int i = 0; i < table->column_count; i++) {
        if (i > 0) {
            printf(" | ");
        }

        printf("%s", table->columns[i].name);
    }

    printf("\n");

    for (int i = 0; i < table->column_count; i++) {
        if (i > 0) {
            printf("-+-");
        }

        printf("--------");
    }

    printf("\n");

    for (int i = 0; i < table->row_count; i++) {
        print_row(table, &table->rows[i]);
    }
}
int find_column(const Table *table, const char *column_name) {
    for (int i = 0; i < table->column_count; i++) {
        if (strcmp(table->columns[i].name, column_name) == 0) {
            return i;
        }
    }

    return -1;
}
void print_column(const Table *table, int column_index) {
    printf("%s\n", table->columns[column_index].name);
    printf("----------------\n");

    for (int i = 0; i < table->row_count; i++) {
        const Value *value = &table->rows[i].values[column_index];

        if (value->type == TYPE_INT) {
            printf("%d\n", value->data.int_value);
        }
        else if (value->type == TYPE_STRING) {
            printf("%s\n", value->data.string_value);
        }
        else if (value->type == TYPE_FLOAT) {
            printf("%.2f\n", value->data.float_value);
        }
    }
}

void print_selected_columns(const Table *table, const int column_indices[],
                            int selected_count) {
    /* Print the selected column names */
    for (int i = 0; i < selected_count; i++) {
        if (i > 0) {
            printf(" | ");
        }

        printf("%s", table->columns[column_indices[i]].name);
    }

    printf("\n");

    /* Print a separator */
    for (int i = 0; i < selected_count; i++) {
        if (i > 0) {
            printf("-+-");
        }

        printf("--------");
    }

    printf("\n");

    /* Print the selected values from every row */
    for (int row_index = 0; row_index < table->row_count; row_index++) {
        for (int col_index = 0; col_index < selected_count; col_index++) {
            if (col_index > 0) {
                printf(" | ");
            }

            const Value *value =
                &table->rows[row_index].values[column_indices[col_index]];

            if (value->type == TYPE_INT) {
                printf("%d", value->data.int_value);
            }
            else if (value->type == TYPE_STRING) {
                printf("%s", value->data.string_value);
            }
            else if (value->type == TYPE_FLOAT) {
                printf("%.2f", value->data.float_value);
            }
        }

        printf("\n");
    }
}

int row_matches_condition(const Table *table, const Row *row,
                          const WhereCondition *condition) {
    int column_index = find_column(table, condition->column_name);

    if (column_index == -1) {
        return 0;
    }

    const Value *value = &row->values[column_index];

    if (value->type == TYPE_INT) {
        char *end;
        long parsed_value = strtol(condition->value, &end, 10);

        if (end == condition->value || *end != '\0') {
            return 0;
        }

        if (parsed_value < INT_MIN || parsed_value > INT_MAX) {
            return 0;
        }

        int expected = (int)parsed_value;

        if (strcmp(condition->operator, "=") == 0)
            return value->data.int_value == expected;
        if (strcmp(condition->operator, "!=") == 0)
            return value->data.int_value != expected;
        if (strcmp(condition->operator, ">") == 0)
            return value->data.int_value > expected;
        if (strcmp(condition->operator, "<") == 0)
            return value->data.int_value < expected;
        if (strcmp(condition->operator, ">=") == 0)
            return value->data.int_value >= expected;
        if (strcmp(condition->operator, "<=") == 0)
            return value->data.int_value <= expected;
    }
    else if (value->type == TYPE_FLOAT) {
        char *end;
        float expected = strtof(condition->value, &end);

        if (end == condition->value || *end != '\0') {
            return 0;
        }

        if (strcmp(condition->operator, "=") == 0)
            return value->data.float_value == expected;
        if (strcmp(condition->operator, "!=") == 0)
            return value->data.float_value != expected;
        if (strcmp(condition->operator, ">") == 0)
            return value->data.float_value > expected;
        if (strcmp(condition->operator, "<") == 0)
            return value->data.float_value < expected;
        if (strcmp(condition->operator, ">=") == 0)
            return value->data.float_value >= expected;
        if (strcmp(condition->operator, "<=") == 0)
            return value->data.float_value <= expected;
    }
    else if (value->type == TYPE_STRING) {
        int comparison = strcmp(value->data.string_value, condition->value);

        if (strcmp(condition->operator, "=") == 0)
            return comparison == 0;
        if (strcmp(condition->operator, "!=") == 0)
            return comparison != 0;
        if (strcmp(condition->operator, ">") == 0)
            return comparison > 0;
        if (strcmp(condition->operator, "<") == 0)
            return comparison < 0;
        if (strcmp(condition->operator, ">=") == 0)
            return comparison >= 0;
        if (strcmp(condition->operator, "<=") == 0)
            return comparison <= 0;
    }

    return 0;
}

int update_rows(Table *table, const char *column_name, const char *new_value,
                const WhereCondition *condition) {
    int column_index = find_column(table, column_name);

    if (column_index == -1) {
        return 0;
    }

    ColumnType type = table->columns[column_index].type;
    int updated_count = 0;

    for (int i = 0; i < table->row_count; i++) {
        Row *row = &table->rows[i];

        if (!row_matches_condition(table, row, condition)) {
            continue;
        }

        Value *value = &row->values[column_index];

        if (type == TYPE_INT) {
            char *end;
            long parsed = strtol(new_value, &end, 10);

            if (end == new_value || *end != '\0' || parsed < INT_MIN ||
                parsed > INT_MAX) {
                return 0;
            }

            value->data.int_value = (int)parsed;
        }
        else if (type == TYPE_FLOAT) {
            char *end;
            float parsed = strtof(new_value, &end);

            if (end == new_value || *end != '\0') {
                return 0;
            }

            value->data.float_value = parsed;
        }
        else if (type == TYPE_STRING) {
            strncpy(value->data.string_value, new_value, MAX_NAME_LENGTH - 1);
            value->data.string_value[MAX_NAME_LENGTH - 1] = '\0';
        }

        updated_count++;
    }

    return updated_count;
}
int delete_rows(Table *table, const WhereCondition *condition) {
    int deleted_count = 0;
    int i = 0;

    while (i < table->row_count) {
        if (row_matches_condition(table, &table->rows[i], condition)) {
            for (int j = i; j < table->row_count - 1; j++) {
                table->rows[j] = table->rows[j + 1];
            }

            table->row_count--;
            deleted_count++;
        }
        else {
            i++;
        }
    }

    return deleted_count;
}
int sort_table(Table *table, const char *column_name, int descending) {
    int column_index = find_column(table, column_name);

    if (column_index == -1) {
        return 0;
    }

    for (int i = 0; i < table->row_count - 1; i++) {
        for (int j = 0; j < table->row_count - i - 1; j++) {
            Value *a = &table->rows[j].values[column_index];
            Value *b = &table->rows[j + 1].values[column_index];

            int comparison = 0;

            if (a->type == TYPE_INT) {
                if (a->data.int_value > b->data.int_value)
                    comparison = 1;
                else if (a->data.int_value < b->data.int_value)
                    comparison = -1;
            }
            else if (a->type == TYPE_FLOAT) {
                if (a->data.float_value > b->data.float_value)
                    comparison = 1;
                else if (a->data.float_value < b->data.float_value)
                    comparison = -1;
            }
            else if (a->type == TYPE_STRING) {
                comparison = strcmp(a->data.string_value, b->data.string_value);
            }

            int should_swap = descending ? (comparison < 0) : (comparison > 0);

            if (should_swap) {
                Row temp = table->rows[j];
                table->rows[j] = table->rows[j + 1];
                table->rows[j + 1] = temp;
            }
        }
    }

    return 1;
}
