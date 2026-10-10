
#include <stdio.h>

#include "storage.h"

#define STORAGE_FILE "minidb.dat"

int save_tables(const Table tables[], int table_count) {
    FILE *file = fopen(STORAGE_FILE, "wb");

    if (file == NULL) {
        return 0;
    }

    if (fwrite(&table_count, sizeof(int), 1, file) != 1) {
        fclose(file);
        return 0;
    }

    if (fwrite(tables, sizeof(Table), (size_t)table_count, file) !=
        (size_t)table_count) {
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}

int load_tables(Table tables[], int *table_count) {
    FILE *file = fopen(STORAGE_FILE, "rb");

    if (file == NULL) {
        *table_count = 0;
        return 1;
    }

    if (fread(table_count, sizeof(int), 1, file) != 1 || *table_count < 0 ||
        *table_count > MAX_TABLES) {
        fclose(file);
        *table_count = 0;
        return 0;
    }

    if (fread(tables, sizeof(Table), (size_t)*table_count, file) !=
        (size_t)*table_count) {
        fclose(file);
        *table_count = 0;
        return 0;
    }

    fclose(file);
    return 1;
}
