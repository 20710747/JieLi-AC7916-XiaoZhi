#ifndef WEBSOCKET_JSON_UTILS_H
#define WEBSOCKET_JSON_UTILS_H

#include "cJSON.h"
#include <stdbool.h>
#include <stdint.h>

// 消息类型枚举
typedef enum {
    MSG_TYPE_HELLO = 0,
    MSG_TYPE_LISTEN,
    MSG_TYPE_ABORT,
    MSG_TYPE_STT,
    MSG_TYPE_LLM,
    MSG_TYPE_TTS,
    MSG_TYPE_MCP,
    MSG_TYPE_SYSTEM,
    MSG_TYPE_CUSTOM,
    MSG_TYPE_UNKNOWN
} message_type_t;

// 监听状态枚举
typedef enum {
    LISTEN_STATE_START,
    LISTEN_STATE_STOP,
    LISTEN_STATE_DETECT
} listen_state_t;

// 监听模式枚举
typedef enum {
    LISTEN_MODE_AUTO,
    LISTEN_MODE_MANUAL,
    LISTEN_MODE_REALTIME
} listen_mode_t;

// TTS 状态枚举
typedef enum {
    TTS_STATE_START,
    TTS_STATE_STOP,
    TTS_STATE_SENTENCE_START
} tts_state_t;

// 系统命令枚举
typedef enum {
    SYSTEM_CMD_REBOOT
} system_command_t;

// 音频参数结构体
typedef struct {
    char format[32];
    uint32_t sample_rate;
    uint8_t channels;
    uint32_t frame_duration;
} audio_params_t;

// 功能特性结构体
typedef struct {
    bool mcp;
    // 可以根据需要添加其他特性字段
} features_t;

// 通用消息结构体
typedef struct {
    message_type_t type;
    char session_id[64];
    uint32_t version;
    union {
        struct {
            features_t features;
            char transport[32];
            audio_params_t audio_params;
        } hello;

        struct {
            listen_state_t state;
            listen_mode_t mode;
            char text[256];
        } listen;

        struct {
            char reason[64];
        } abort;

        struct {
            char text[512];
        } stt;

        struct {
            char emotion[32];
            char text[128];
        } llm;

        struct {
            tts_state_t state;
            char text[512];
        } tts;

        struct {
            cJSON* payload;
        } mcp;

        struct {
            system_command_t command;
        } system;

        struct {
            cJSON* payload;
        } custom;
    } data;
} websocket_message_t;

// ==================== 创建消息的函数 ====================

/**
 * @brief 创建 Hello 消息
 * @param version 协议版本
 * @param features 功能特性
 * @param transport 传输协议
 * @param audio_params 音频参数
 * @return cJSON* 创建的 JSON 对象
 */
cJSON* create_hello_message(uint32_t version, const features_t* features,
                           const char* transport, const audio_params_t* audio_params);

/**
 * @brief 创建 Listen 消息
 * @param session_id 会话ID
 * @param state 监听状态
 * @param mode 监听模式
 * @param text 唤醒词文本（可选）
 * @return cJSON* 创建的 JSON 对象
 */
cJSON* create_listen_message(const char* session_id, listen_state_t state,
                            listen_mode_t mode, const char* text);

/**
 * @brief 创建 Abort 消息
 * @param session_id 会话ID
 * @param reason 终止原因
 * @return cJSON* 创建的 JSON 对象
 */
cJSON* create_abort_message(const char* session_id, const char* reason);

/**
 * @brief 创建 Wake Word Detected 消息
 * @param session_id 会话ID
 * @param wake_word 唤醒词
 * @return cJSON* 创建的 JSON 对象
 */
cJSON* create_wake_word_message(const char* session_id, const char* wake_word);

/**
 * @brief 创建 MCP 消息
 * @param session_id 会话ID
 * @param payload MCP payload（JSON-RPC 2.0格式）
 * @return cJSON* 创建的 JSON 对象
 */
cJSON* create_mcp_message(const char* session_id, cJSON* payload);

/**
 * @brief 创建 STT 消息（服务器端使用）
 * @param session_id 会话ID
 * @param text 识别文本
 * @return cJSON* 创建的 JSON 对象
 */
cJSON* create_stt_message(const char* session_id, const char* text);

/**
 * @brief 创建 LLM 消息（服务器端使用）
 * @param session_id 会话ID
 * @param emotion 表情
 * @param text 文本
 * @return cJSON* 创建的 JSON 对象
 */
cJSON* create_llm_message(const char* session_id, const char* emotion, const char* text);

/**
 * @brief 创建 TTS 消息
 * @param session_id 会话ID
 * @param state TTS状态
 * @param text 文本内容（对于sentence_start状态）
 * @return cJSON* 创建的 JSON 对象
 */
cJSON* create_tts_message(const char* session_id, tts_state_t state, const char* text);

/**
 * @brief 创建 System 消息
 * @param session_id 会话ID
 * @param command 系统命令
 * @return cJSON* 创建的 JSON 对象
 */
cJSON* create_system_message(const char* session_id, system_command_t command);

// ==================== 解析消息的函数 ====================

/**
 * @brief 解析收到的 JSON 消息
 * @param json_str JSON 字符串
 * @param message 输出解析后的消息结构体
 * @return bool 解析是否成功
 */
bool parse_websocket_message(const char* json_str, websocket_message_t* message);

/**
 * @brief 从 cJSON 对象解析消息
 * @param json cJSON 对象
 * @param message 输出解析后的消息结构体
 * @return bool 解析是否成功
 */
bool parse_message_from_json(cJSON* json, websocket_message_t* message);

/**
 * @brief 释放消息结构体中动态分配的内存
 * @param message 消息结构体
 */
void free_websocket_message(websocket_message_t* message);

// ==================== 工具函数 ====================

/**
 * @brief 将消息类型转换为字符串
 * @param type 消息类型
 * @return const char* 类型字符串
 */
const char* message_type_to_string(message_type_t type);

/**
 * @brief 将字符串转换为消息类型
 * @param type_str 类型字符串
 * @return message_type_t 消息类型
 */
message_type_t string_to_message_type(const char* type_str);

/**
 * @brief 创建音频参数对象
 * @param params 音频参数结构体
 * @return cJSON* 音频参数 JSON 对象
 */
cJSON* create_audio_params_object(const audio_params_t* params);

/**
 * @brief 解析音频参数对象
 * @param json 音频参数 JSON 对象
 * @param params 输出音频参数结构体
 * @return bool 解析是否成功
 */
bool parse_audio_params_object(cJSON* json, audio_params_t* params);

/**
 * @brief 创建功能特性对象
 * @param features 功能特性结构体
 * @return cJSON* 功能特性 JSON 对象
 */
cJSON* create_features_object(const features_t* features);

/**
 * @brief 检查消息是否包含必需的字段
 * @param json JSON 对象
 * @return bool 是否包含必需字段
 */
bool validate_message_format(cJSON* json);



void example_create_hello(void);// 示例：创建和发送 Hello 消息
void example_parse_message(const char* received_json);// 示例：解析收到的消息
void example_create_mcp(void);// 示例：创建 MCP 消息


#endif // WEBSOCKET_JSON_UTILS_H
