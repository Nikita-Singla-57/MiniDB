#include <stdio.h>
#include <string.h>

#include "cli.h"
#include "parser.h"
#include "storage.h"
#include "table.h"
void start_cli(void) {
    char input[500];

    Table tables[MAX_TABLES];
    int table_count = 0;

    if (!load_tables(tables, &table_count)) {
        printf("Warning: Could not load saved data.\n");
        table_count = 0;
    }

    printf("Welcome to MiniDB!\n");
    printf("Type \".help\" for help.\n\n");

    while (1) {
        printf("MiniDB> ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, ".exit") == 0) {
            printf("Goodbye!\n");
            break;
        }
        else if (strcmp(input, ".help") == 0) {
            printf("Available commands:\n");
            printf(".help     Show this help message\n");
            printf(".version  Show MiniDB version\n");
            printf(".exit     Exit MiniDB\n");
        }
        else if (strcmp(input, ".test_table") == 0) {
            Table table;

            init_table(&table, "students");
            add_column(&table, "id", TYPE_INT);
            add_column(&table, "name", TYPE_STRING);
            add_column(&table, "age", TYPE_INT);

            print_table(&table);
        }
        else if (strncmp(input, "DROP TABLE ", 11) == 0) {
            char table_name[MAX_NAME_LENGTH];

            if (!parse_drop_table(input, table_name)) {
                printf("Error: invalid DROP TABLE statement.\n");
                continue;
            }

            int table_index = -1;

            for (int i = 0; i < table_count; i++) {
                if (strcmp(tables[i].name, table_name) == 0) {
                    table_index = i;
                    break;
                }
            }

            if (table_index == -1) {
                printf("Error: table '%s' does not exist.\n", table_name);
                continue;
            }

            for (int i = table_index; i < table_count - 1; i++) {
                tables[i] = tables[i + 1];
            }

            table_count--;
            printf("Table '%s' dropped.\n", table_name);

            if (!save_tables(tables, table_count)) {
                printf("Warning: Could not save database.\n");
            }
        }
        else if (strncmp(input, "CREATE TABLE ", 12) == 0) {
            if (table_count >= MAX_TABLES) {
                printf("Error: maximum number of tables reached.\n");
                continue;
            }

            if (parse_create_table(input, &tables[table_count])) {
                printf("Table '%s' created.\n", tables[table_count].name);
                table_count++;
                if (!save_tables(tables, table_count)) {
                    printf("Warning: Could not save database.\n");
                }
            }
            else {
                printf("Error: invalid CREATE TABLE statement.\n");
            }
        }
        else if (strncmp(input, "INSERT INTO", 11) == 0) {
            char table_name[MAX_NAME_LENGTH];
            char values_text[500];
            char tokens[MAX_COLUMNS][MAX_NAME_LENGTH];
            int token_count;
            Row row;

            if (!parse_insert(input, table_name, values_text)) {
                printf("Error: invalid INSERT statement.\n");
                continue;
            }

            Table *table = find_table(tables, table_count, table_name);

            if (table == NULL) {
                printf("Error: table '%s' does not exist.\n", table_name);
                continue;
            }

            if (!parse_values(values_text, tokens, &token_count)) {
                printf("Error: invalid values.\n");
                continue;
            }

            if (!build_row(table, tokens, token_count, &row)) {
                printf("Error: values do not match table columns.\n");
                continue;
            }

            if (!add_row(table, &row)) {
                printf("Error: table is full.\n");
                continue;
            }

            printf("Row inserted.\n");
            if (!save_tables(tables, table_count)) {
                printf("Warning: Could not save database.\n");
            }
        }
        else if (strncmp(input, "UPDATE ", 7) == 0) {
            char table_name[MAX_NAME_LENGTH];
            char column_name[MAX_NAME_LENGTH];
            char new_value[MAX_NAME_LENGTH];
            WhereCondition condition;

            if (!parse_update(input, table_name, column_name, new_value)) {
                printf("Error: invalid UPDATE statement.\n");
                continue;
            }

            if (!parse_where_condition(input, &condition)) {
                printf("Error: invalid WHERE condition.\n");
                continue;
            }

            Table *table = find_table(tables, table_count, table_name);

            if (table == NULL) {
                printf("Error: table '%s' does not exist.\n", table_name);
                continue;
            }

            if (find_column(table, column_name) == -1) {
                printf("Error: column '%s' does not exist.\n", column_name);
                continue;
            }

            if (find_column(table, condition.column_name) == -1) {
                printf("Error: column '%s' does not exist.\n",
                       condition.column_name);
                continue;
            }

            int updated =
                update_rows(table, column_name, new_value, &condition);

            if (updated > 0) {
                printf("%d row(s) updated.\n", updated);

                if (!save_tables(tables, table_count)) {
                    printf("Warning: Could not save database.\n");
                }
            }
            else {
                printf("No rows updated. Check the condition and value.\n");
            }
        }
        else if (strncmp(input, "DELETE FROM ", 12) == 0) {
            char table_name[MAX_NAME_LENGTH];
            WhereCondition condition;

            if (!parse_delete(input, table_name)) {
                printf("Error: invalid DELETE statement.\n");
                continue;
            }

            if (!parse_where_condition(input, &condition)) {
                printf("Error: invalid WHERE condition.\n");
                continue;
            }

            Table *table = find_table(tables, table_count, table_name);

            if (table == NULL) {
                printf("Error: table '%s' does not exist.\n", table_name);
                continue;
            }

            if (find_column(table, condition.column_name) == -1) {
                printf("Error: column '%s' does not exist.\n",
                       condition.column_name);
                continue;
            }

            int deleted = delete_rows(table, &condition);

            if (deleted > 0) {
                printf("%d row(s) deleted.\n", deleted);

                if (!save_tables(tables, table_count)) {
                    printf("Warning: Could not save database.\n");
                }
            }
            else {
                printf("No matching rows found.\n");
            }
        }
        else if (strncmp(input, "SELECT ", 7) == 0) {
            char base_query[sizeof(input)];
            char table_name[MAX_NAME_LENGTH];
            char order_column[MAX_NAME_LENGTH];
            int descending = 0;

            const char *order_start = strstr(input, " ORDER BY ");
            const char *where_start = strstr(input, " WHERE ");
            int has_order = (order_start != NULL);
            int has_where = (where_start != NULL);

            WhereCondition condition;

            if (has_order &&
                !parse_order_by(input, order_column, &descending)) {
                printf("Error: invalid ORDER BY clause.\n");
                continue;
            }

            /*
             * Remove ORDER BY from the query first.
             * Then separate the SELECT query from WHERE.
             */
            const char *query_end =
                order_start != NULL ? order_start : input + strlen(input);

            const char *where_in_query = NULL;

            if (where_start != NULL && where_start < query_end) {
                where_in_query = where_start;
            }

            if (where_in_query != NULL) {
                size_t base_length = (size_t)(where_in_query - input);
                memcpy(base_query, input, base_length);
                base_query[base_length] = '\0';

                if (!parse_where_condition(input, &condition)) {
                    printf("Error: invalid WHERE condition.\n");
                    continue;
                }
            }
            else {
                size_t base_length = (size_t)(query_end - input);
                memcpy(base_query, input, base_length);
                base_query[base_length] = '\0';
            }

            /*
             * SELECT * FROM ...
             */
            if (strncmp(base_query, "SELECT * FROM ", 14) == 0) {
                if (!parse_select(base_query, table_name)) {
                    printf("Error: invalid SELECT statement.\n");
                    continue;
                }

                Table *table = find_table(tables, table_count, table_name);

                if (table == NULL) {
                    printf("Error: table '%s' does not exist.\n", table_name);
                    continue;
                }

                /*
                 * Sort a temporary copy, preserving the original table.
                 */
                Table sorted_table;

                if (has_order) {
                    if (find_column(table, order_column) == -1) {
                        printf("Error: column '%s' does not exist.\n",
                               order_column);
                        continue;
                    }

                    sorted_table = *table;

                    if (!sort_table(&sorted_table, order_column, descending)) {
                        printf("Error: could not sort table.\n");
                        continue;
                    }

                    table = &sorted_table;
                }

                if (has_where) {
                    if (find_column(table, condition.column_name) == -1) {
                        printf("Error: column '%s' does not exist.\n",
                               condition.column_name);
                        continue;
                    }

                    int matched = 0;

                    for (int i = 0; i < table->row_count; i++) {
                        if (row_matches_condition(table, &table->rows[i],
                                                  &condition)) {
                            print_row(table, &table->rows[i]);
                            matched = 1;
                        }
                    }

                    if (!matched) {
                        printf("No matching rows found.\n");
                    }
                }
                else {
                    print_table_rows(table);
                }
            }
            /*
             * SELECT column1, column2 FROM ...
             */
            else {
                char columns_text[MAX_NAME_LENGTH];

                if (!parse_select_columns(base_query, columns_text,
                                          table_name)) {
                    printf("Error: invalid SELECT statement.\n");
                    continue;
                }

                Table *table = find_table(tables, table_count, table_name);

                if (table == NULL) {
                    printf("Error: table '%s' does not exist.\n", table_name);
                    continue;
                }

                int column_indices[MAX_COLUMNS];
                int selected_count = 0;
                size_t columns_length = strlen(columns_text);

                while (columns_length > 0 &&
                       columns_text[columns_length - 1] == ' ') {
                    columns_length--;
                }

                if (columns_length == 0 ||
                    columns_text[columns_length - 1] == ',') {
                    printf("Error: invalid column list.\n");
                    continue;
                }

                char *token = strtok(columns_text, ",");
                int valid_columns = 1;

                while (token != NULL) {
                    while (*token == ' ') {
                        token++;
                    }

                    char *end = token + strlen(token);

                    while (end > token && end[-1] == ' ') {
                        end--;
                    }

                    *end = '\0';

                    if (*token == '\0' || selected_count >= MAX_COLUMNS) {
                        printf("Error: invalid column list.\n");
                        valid_columns = 0;
                        break;
                    }

                    int index = find_column(table, token);

                    if (index == -1) {
                        printf("Error: column '%s' does not exist.\n", token);
                        valid_columns = 0;
                        break;
                    }

                    column_indices[selected_count++] = index;
                    token = strtok(NULL, ",");
                }

                if (!valid_columns || selected_count == 0) {
                    continue;
                }

                if (has_order) {
                    if (find_column(table, order_column) == -1) {
                        printf("Error: column '%s' does not exist.\n",
                               order_column);
                        continue;
                    }

                    Table sorted_table = *table;

                    if (!sort_table(&sorted_table, order_column, descending)) {
                        printf("Error: could not sort table.\n");
                        continue;
                    }

                    table = &sorted_table;
                }

                if (has_where) {
                    if (find_column(table, condition.column_name) == -1) {
                        printf("Error: column '%s' does not exist.\n",
                               condition.column_name);
                        continue;
                    }

                    Table filtered_table = *table;
                    filtered_table.row_count = 0;

                    for (int i = 0; i < table->row_count; i++) {
                        if (row_matches_condition(table, &table->rows[i],
                                                  &condition)) {
                            filtered_table.rows[filtered_table.row_count++] =
                                table->rows[i];
                        }
                    }

                    if (filtered_table.row_count == 0) {
                        printf("No matching rows found.\n");
                    }
                    else {
                        print_selected_columns(&filtered_table, column_indices,
                                               selected_count);
                    }
                }
                else {
                    print_selected_columns(table, column_indices,
                                           selected_count);
                }
            }
        }
        else if (strcmp(input, ".test_build_row") == 0) {
            Table table;
            Row row;

            char tokens[MAX_COLUMNS][MAX_NAME_LENGTH] = {"1", "Nikita", "20"};

            init_table(&table, "students");
            add_column(&table, "id", TYPE_INT);
            add_column(&table, "name", TYPE_STRING);
            add_column(&table, "age", TYPE_INT);

            if (build_row(&table, tokens, 3, &row)) {
                printf("Row built successfully.\n");
                printf("ID: %d\n", row.values[0].data.int_value);
                printf("Name: %s\n", row.values[1].data.string_value);
                printf("Age: %d\n", row.values[2].data.int_value);
            }
            else {
                printf("Failed to build row.\n");
            }
        }
        else if (strcmp(input, ".test_print_row") == 0) {
            Table table;
            Row row;

            init_table(&table, "students");
            add_column(&table, "id", TYPE_INT);
            add_column(&table, "name", TYPE_STRING);
            add_column(&table, "age", TYPE_INT);

            row.values[0].type = TYPE_INT;
            row.values[0].data.int_value = 1;

            row.values[1].type = TYPE_STRING;
            strcpy(row.values[1].data.string_value, "Nikita");

            row.values[2].type = TYPE_INT;
            row.values[2].data.int_value = 20;

            print_row(&table, &row);
        }
        else if (strcmp(input, ".test_select") == 0) {
            Table table;
            Row row;

            init_table(&table, "students");
            add_column(&table, "id", TYPE_INT);
            add_column(&table, "name", TYPE_STRING);
            add_column(&table, "age", TYPE_INT);

            row.values[0].type = TYPE_INT;
            row.values[0].data.int_value = 1;

            row.values[1].type = TYPE_STRING;
            strcpy(row.values[1].data.string_value, "Nikita");

            row.values[2].type = TYPE_INT;
            row.values[2].data.int_value = 20;

            add_row(&table, &row);

            row.values[0].data.int_value = 2;
            strcpy(row.values[1].data.string_value, "Rahul");
            row.values[2].data.int_value = 21;

            add_row(&table, &row);

            row.values[0].data.int_value = 3;
            strcpy(row.values[1].data.string_value, "Aman");
            row.values[2].data.int_value = 19;

            add_row(&table, &row);

            print_table_rows(&table);
        }
        else if (strcmp(input, ".test_select_parser") == 0) {
            char table_name[MAX_NAME_LENGTH];
            const char *test_command = "SELECT * FROM students";

            if (parse_select(test_command, table_name)) {
                printf("SELECT parser succeeded.\n");
                printf("Table: %s\n", table_name);
            }
            else {
                printf("SELECT parser failed.\n");
            }
        }
        else if (strcmp(input, ".version") == 0) {
            printf("MiniDB version 1.0\n");
        }
        else if (strlen(input) == 0) {
            continue;
        }
        else if (strcmp(input, ".test_row") == 0) {
            Table table;
            Row row;

            init_table(&table, "students");
            add_column(&table, "id", TYPE_INT);
            add_column(&table, "name", TYPE_STRING);
            add_column(&table, "age", TYPE_INT);

            row.values[0].type = TYPE_INT;
            row.values[0].data.int_value = 1;

            row.values[1].type = TYPE_STRING;
            strcpy(row.values[1].data.string_value, "Nikita");

            row.values[2].type = TYPE_INT;
            row.values[2].data.int_value = 20;

            if (add_row(&table, &row)) {
                printf("Row added successfully.\n");
                printf("Row count: %d\n", table.row_count);
            }
            else {
                printf("Failed to add row.\n");
            }
        }
        else if (strcmp(input, ".test_insert") == 0) {
            char table_name[MAX_NAME_LENGTH];
            char values_text[500];

            const char *test_command =
                "INSERT INTO students VALUES (1, \"Nikita\", 20)";

            if (parse_insert(test_command, table_name, values_text)) {
                printf("INSERT parser succeeded.\n");
                printf("Table: %s\n", table_name);
                printf("Values: %s\n", values_text);
            }
            else {
                printf("INSERT parser failed.\n");
            }
        }
        else if (strcmp(input, ".test_values") == 0) {
            char tokens[MAX_COLUMNS][MAX_NAME_LENGTH];
            int count;

            const char *test_values = "1, \"Nikita\", 20";

            if (parse_values(test_values, tokens, &count)) {
                printf("Values parsed successfully.\n");

                for (int i = 0; i < count; i++) {
                    printf("Value %d: %s\n", i + 1, tokens[i]);
                }
            }
            else {
                printf("Failed to parse values.\n");
            }
        }
        else {
            printf("Unknown command: %s\n", input);
        }
    }
}
