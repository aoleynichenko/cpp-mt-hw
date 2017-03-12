#include "data_type.h"

int data_less(const void *a, const void *b)
{
    return *(data_type*)a < *(data_type*)b;
}
