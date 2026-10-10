#ifndef STORAGE_H
#define STORAGE_H

#include "table.h"

int save_tables(const Table tables[], int table_count);
int load_tables(Table tables[], int *table_count);

#endif

