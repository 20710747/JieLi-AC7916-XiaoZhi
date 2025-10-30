#include "robot_util.h"


// 将字符串转换为小写
void to_lower(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

int map_string_to_value(const char* input, const StringValueMap* map, int map_size) {
    char lower_input[64]; // 假设输入字符串不超过63个字符
    if(strlen(input)>=64)
        return -1;
    strcpy(lower_input, input);
    to_lower(lower_input);

    for (int i = 0; i < map_size; i++) {
        if (strcmp(lower_input, map[i].str) == 0) {
            return map[i].value;
        }
    }
    return -1;
}

int hex_string_to_uint8_array(const char *input, uint8_t *output) {
    int len = strlen(input);

    // 检查字符串长度是否为偶数
    if (len % 2 != 0) {
        return 0;
    }

    int output_len = len / 2;

    for (int i = 0; i < output_len; i++) {
        char high_char = input[2 * i];
        char low_char = input[2 * i + 1];

        // 检查字符是否为有效的十六进制字符
        if (!isxdigit(high_char) || !isxdigit(low_char)) {
            return 0;
        }

        // 转换高4位
        uint8_t high_nibble;
        if (isdigit(high_char)) {
            high_nibble = high_char - '0';
        } else {
            high_nibble = toupper(high_char) - 'A' + 10;
        }

        // 转换低4位
        uint8_t low_nibble;
        if (isdigit(low_char)) {
            low_nibble = low_char - '0';
        } else {
            low_nibble = toupper(low_char) - 'A' + 10;
        }

        // 合并为1个字节
        output[i] = (high_nibble << 4) | low_nibble;
    }

    return output_len;
}
