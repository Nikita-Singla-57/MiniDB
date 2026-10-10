

#include <stdio.h>
#include <string.h>

#include "cli.h"
#include "parser.h"
#include "table.h"

void start_cli(void) {
    char input[500];

    Table tables[MAX_TABLES];
    int table_count = 0;

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
        else if (strncmp(input, "CREATE TABLE ", 12) == 0) {
            if (table_count >= MAX_TABLES) {
                printf("Error: maximum number of tables reached.\n");
                continue;
            }

            if (parse_create_table(input, &tables[table_count])) {
                printf("Table '%s' created.\n", tables[table_count].name);

                table_count++;
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
        }
        else if (strncmp(input, "SELECT ", 7) == 0) {
            if (strncmp(input, "SELECT * FROM ", 14) == 0) {
                char table_name[MAX_NAME_LENGTH];

                if (!parse_select(input, table_name)) {
                    printf("Error: invalid SELECT statement.\n");
                    continue;
                }

                Table *table = find_table(tables, table_count, table_name);

                if (table == NULL) {
                    printf("Error: table '%s' does not exist.\n", table_name);
                    continue;
                }
                print_table_rows(table);
            }
            else {
                char columns_text[MAX_NAME_LENGTH];
                char table_name[MAX_NAME_LENGTH];

                if (!parse_select_columns(input, columns_text, table_name)) {
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

                    char *end = token + strlen(token) - 1;

                    while (end >= token && *end == ' ') {
                        *end = '\0';
                        end--;
                    }

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

                if (valid_columns && token == NULL && selected_count > 0) {
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
