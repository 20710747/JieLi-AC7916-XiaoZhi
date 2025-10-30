#ifndef __ROBOT_UTIL_H__

#include "system/includes.h"

typedef struct {
    const char* str;
    int value;
} StringValueMap;

int map_string_to_value(const char* input, const StringValueMap* map, int map_size);

int hex_string_to_uint8_array(const char *input, uint8_t *output);


#define MAP_STRING2VALUE(str, mappings) \
    map_string_to_value((str), (mappings), sizeof(mappings) / sizeof((mappings)[0]))

#endif // __ROBOT_UTIL_H__
