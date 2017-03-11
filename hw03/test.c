#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int compare(const uint32_t* a, const uint32_t* b)
{
    return *a > *b;
}

int main()
{
    uint32_t nums[] = {35465, 65436, 22114, 2017, 10023};
    qsort(nums, 5, sizeof(uint32_t), compare);
    for (int i = 0; i < 5; i++)
        printf("%d\n", nums[i]);
}
