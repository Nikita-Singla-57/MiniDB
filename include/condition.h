#ifndef CONDITION_H
#define CONDITION_H

#include "table.h"

typedef struct WhereCondition {
    char column_name[MAX_NAME_LENGTH];
    char operator[3];
    char value[MAX_NAME_LENGTH];
} WhereCondition;

#endif
