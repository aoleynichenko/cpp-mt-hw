#ifndef DATA_TYPE_H
#define DATA_TYPE_H

#include <stdint.h>

typedef uint32_t data_type;

#ifdef __cplusplus
extern "C" {
#endif  // C++ defined

int compare(const void *, const void *);

#ifdef __cplusplus
}
#endif  // C++ defined

#endif  // ifndef DATA_TYPE_H