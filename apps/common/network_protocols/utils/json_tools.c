#include "json_tools.h"
#include <string.h>
#include <stdio.h>

// ==================== 实现创建消息的函数 ====================

cJSON* create_hello_message(uint32_t version, const features_t* features,
                           const char* transport, const audio_params_t* audio_params) {
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "hello");
    cJSON_AddNumberToObject(root, "version", version);

    if (features) {
        cJSON* features_obj = create_features_object(features);
        cJSON_AddItemToObject(root, "features", features_obj);
    }

    if (transport) {
        cJSON_AddStringToObject(root, "transport", transport);
    }

    if (audio_params) {
        cJSON* audio_params_obj = create_audio_params_object(audio_params);
        cJSON_AddItemToObject(root, "audio_params", audio_params_obj);
    }

    return root;
}

cJSON* create_listen_message(const char* session_id, listen_state_t state,
                            listen_mode_t mode, const char* text) {
    cJSON* root = cJSON_CreateObject();

    if (session_id) {
        cJSON_AddStringToObject(root, "session_id", session_id);
    }

    cJSON_AddStringToObject(root, "type", "listen");

    // 添加状态
    const char* state_str = "";
    switch (state) {
        case LISTEN_STATE_START: state_str = "start"; break;
        case LISTEN_STATE_STOP: state_str = "stop"; break;
        case LISTEN_STATE_DETECT: state_str = "detect"; break;
    }
    cJSON_AddStringToObject(root, "state", state_str);

    // 添加模式
    const char* mode_str = "";
    switch (mode) {
        case LISTEN_MODE_AUTO: mode_str = "auto"; break;
        case LISTEN_MODE_MANUAL: mode_str = "manual"; break;
        case LISTEN_MODE_REALTIME: mode_str = "realtime"; break;
    }
    cJSON_AddStringToObject(root, "mode", mode_str);

    if (text && strlen(text) > 0) {
        cJSON_AddStringToObject(root, "text", text);
    }

    return root;
}

cJSON* create_abort_message(const char* session_id, const char* reason) {
    cJSON* root = cJSON_CreateObject();

    if (session_id) {
        cJSON_AddStringToObject(root, "session_id", session_id);
    }

    cJSON_AddStringToObject(root, "type", "abort");

    if (reason) {
        cJSON_AddStringToObject(root, "reason", reason);
    }

    return root;
}

cJSON* create_wake_word_message(const char* session_id, const char* wake_word) {
    return create_listen_message(session_id, LISTEN_STATE_DETECT,
                                LISTEN_MODE_AUTO, wake_word);
}

cJSON* create_mcp_message(const char* session_id, cJSON* payload) {
    cJSON* root = cJSON_CreateObject();

    if (session_id) {
        cJSON_AddStringToObject(root, "session_id", session_id);
    }

    cJSON_AddStringToObject(root, "type", "mcp");

    if (payload) {
        cJSON_AddItemToObject(root, "payload", payload);
    }

    return root;
}

cJSON* create_stt_message(const char* session_id, const char* text) {
    cJSON* root = cJSON_CreateObject();

    if (session_id) {
        cJSON_AddStringToObject(root, "session_id", session_id);
    }

    cJSON_AddStringToObject(root, "type", "stt");

    if (text) {
        cJSON_AddStringToObject(root, "text", text);
    }

    return root;
}

cJSON* create_llm_message(const char* session_id, const char* emotion, const char* text) {
    cJSON* root = cJSON_CreateObject();

    if (session_id) {
        cJSON_AddStringToObject(root, "session_id", session_id);
    }

    cJSON_AddStringToObject(root, "type", "llm");

    if (emotion) {
        cJSON_AddStringToObject(root, "emotion", emotion);
    }

    if (text) {
        cJSON_AddStringToObject(root, "text", text);
    }

    return root;
}

cJSON* create_tts_message(const char* session_id, tts_state_t state, const char* text) {
    cJSON* root = cJSON_CreateObject();

    if (session_id) {
        cJSON_AddStringToObject(root, "session_id", session_id);
    }

    cJSON_AddStringToObject(root, "type", "tts");

    const char* state_str = "";
    switch (state) {
        case TTS_STATE_START: state_str = "start"; break;
        case TTS_STATE_STOP: state_str = "stop"; break;
        case TTS_STATE_SENTENCE_START: state_str = "sentence_start"; break;
    }
    cJSON_AddStringToObject(root, "state", state_str);

    if (text && (state == TTS_STATE_SENTENCE_START)) {
        cJSON_AddStringToObject(root, "text", text);
    }

    return root;
}

cJSON* create_system_message(const char* session_id, system_command_t command) {
    cJSON* root = cJSON_CreateObject();

    if (session_id) {
        cJSON_AddStringToObject(root, "session_id", session_id);
    }

    cJSON_AddStringToObject(root, "type", "system");

    const char* command_str = "";
    switch (command) {
        case SYSTEM_CMD_REBOOT: command_str = "reboot"; break;
    }
    cJSON_AddStringToObject(root, "command", command_str);

    return root;
}



// ==================== 实现解析消息的函数 ====================

bool parse_websocket_message(const char* json_str, websocket_message_t* message) {
    if (!json_str || !message) return false;

    cJSON* root = cJSON_Parse(json_str);
    if (!root) {
        return false;
    }

    bool result = parse_message_from_json(root, message);
    cJSON_Delete(root);
    return result;
}

bool parse_message_from_json(cJSON* json, websocket_message_t* message) {
    if (!json || !message) return false;

    memset(message, 0, sizeof(websocket_message_t));

    // 解析消息类型
    cJSON* type_item = cJSON_GetObjectItem(json, "type");
    if (!cJSON_IsString(type_item)) {
        return false;
    }

    message->type = string_to_message_type(type_item->valuestring);

    // 解析会话ID
    cJSON* session_item = cJSON_GetObjectItem(json, "session_id");
    if (cJSON_IsString(session_item)) {
        strncpy(message->session_id, session_item->valuestring,
                sizeof(message->session_id) - 1);
    }

    // 解析版本
    cJSON* version_item = cJSON_GetObjectItem(json, "version");
    if (cJSON_IsNumber(version_item)) {
        message->version = version_item->valueint;
    }

    // 根据消息类型解析具体内容
    switch (message->type) {
        case MSG_TYPE_HELLO: {
            cJSON* features_item = cJSON_GetObjectItem(json, "features");
            if (features_item) {
                cJSON* mcp_item = cJSON_GetObjectItem(features_item, "mcp");
                if (cJSON_IsBool(mcp_item)) {
                    message->data.hello.features.mcp = cJSON_IsTrue(mcp_item);
                }
            }

            cJSON* transport_item = cJSON_GetObjectItem(json, "transport");
            if (cJSON_IsString(transport_item)) {
                strncpy(message->data.hello.transport, transport_item->valuestring,
                       sizeof(message->data.hello.transport) - 1);
            }

            cJSON* audio_params_item = cJSON_GetObjectItem(json, "audio_params");
            if (audio_params_item) {
                parse_audio_params_object(audio_params_item, &message->data.hello.audio_params);
            }
            break;
        }

        case MSG_TYPE_LISTEN: {
            cJSON* state_item = cJSON_GetObjectItem(json, "state");
            if (cJSON_IsString(state_item)) {
                const char* state_str = state_item->valuestring;
                if (strcmp(state_str, "start") == 0) {
                    message->data.listen.state = LISTEN_STATE_START;
                } else if (strcmp(state_str, "stop") == 0) {
                    message->data.listen.state = LISTEN_STATE_STOP;
                } else if (strcmp(state_str, "detect") == 0) {
                    message->data.listen.state = LISTEN_STATE_DETECT;
                }
            }

            cJSON* mode_item = cJSON_GetObjectItem(json, "mode");
            if (cJSON_IsString(mode_item)) {
                const char* mode_str = mode_item->valuestring;
                if (strcmp(mode_str, "auto") == 0) {
                    message->data.listen.mode = LISTEN_MODE_AUTO;
                } else if (strcmp(mode_str, "manual") == 0) {
                    message->data.listen.mode = LISTEN_MODE_MANUAL;
                } else if (strcmp(mode_str, "realtime") == 0) {
                    message->data.listen.mode = LISTEN_MODE_REALTIME;
                }
            }

            cJSON* text_item = cJSON_GetObjectItem(json, "text");
            if (cJSON_IsString(text_item)) {
                strncpy(message->data.listen.text, text_item->valuestring,
                       sizeof(message->data.listen.text) - 1);
            }
            break;
        }

        case MSG_TYPE_ABORT: {
            cJSON* reason_item = cJSON_GetObjectItem(json, "reason");
            if (cJSON_IsString(reason_item)) {
                strncpy(message->data.abort.reason, reason_item->valuestring,
                       sizeof(message->data.abort.reason) - 1);
            }
            break;
        }

        case MSG_TYPE_STT: {
            cJSON* text_item = cJSON_GetObjectItem(json, "text");
            if (cJSON_IsString(text_item)) {
                strncpy(message->data.stt.text, text_item->valuestring,
                       sizeof(message->data.stt.text) - 1);
            }
            break;
        }

        case MSG_TYPE_LLM: {
            cJSON* emotion_item = cJSON_GetObjectItem(json, "emotion");
            if (cJSON_IsString(emotion_item)) {
                strncpy(message->data.llm.emotion, emotion_item->valuestring,
                       sizeof(message->data.llm.emotion) - 1);
            }

            cJSON* text_item = cJSON_GetObjectItem(json, "text");
            if (cJSON_IsString(text_item)) {
                strncpy(message->data.llm.text, text_item->valuestring,
                       sizeof(message->data.llm.text) - 1);
            }
            break;
        }

        case MSG_TYPE_TTS: {
            cJSON* state_item = cJSON_GetObjectItem(json, "state");
            if (cJSON_IsString(state_item)) {
                const char* state_str = state_item->valuestring;
                if (strcmp(state_str, "start") == 0) {
                    message->data.tts.state = TTS_STATE_START;
                } else if (strcmp(state_str, "stop") == 0) {
                    message->data.tts.state = TTS_STATE_STOP;
                } else if (strcmp(state_str, "sentence_start") == 0) {
                    message->data.tts.state = TTS_STATE_SENTENCE_START;
                }
            }

            cJSON* text_item = cJSON_GetObjectItem(json, "text");
            if (cJSON_IsString(text_item)) {
                strncpy(message->data.tts.text, text_item->valuestring,
                       sizeof(message->data.tts.text) - 1);
            }
            break;
        }

        case MSG_TYPE_MCP: {
            cJSON* payload_item = cJSON_GetObjectItem(json, "payload");
            if (payload_item) {
                message->data.mcp.payload = cJSON_Duplicate(payload_item, true);
            }
            break;
        }

        case MSG_TYPE_SYSTEM: {
            cJSON* command_item = cJSON_GetObjectItem(json, "command");
            if (cJSON_IsString(command_item)) {
                const char* command_str = command_item->valuestring;
                if (strcmp(command_str, "reboot") == 0) {
                    message->data.system.command = SYSTEM_CMD_REBOOT;
                }
            }
            break;
        }

        case MSG_TYPE_CUSTOM: {
            cJSON* payload_item = cJSON_GetObjectItem(json, "payload");
            if (payload_item) {
                message->data.custom.payload = cJSON_Duplicate(payload_item, true);
            }
            break;
        }

        default:
            break;
    }

    return true;
}

void free_websocket_message(websocket_message_t* message) {
    if (!message) return;

    switch (message->type) {
        case MSG_TYPE_MCP:
            if (message->data.mcp.payload) {
                cJSON_Delete(message->data.mcp.payload);
                message->data.mcp.payload = NULL;
            }
            break;

        case MSG_TYPE_CUSTOM:
            if (message->data.custom.payload) {
                cJSON_Delete(message->data.custom.payload);
                message->data.custom.payload = NULL;
            }
            break;

        default:
            break;
    }
}

// ==================== 实现工具函数 ====================

const char* message_type_to_string(message_type_t type) {
    switch (type) {
        case MSG_TYPE_HELLO: return "hello";
        case MSG_TYPE_LISTEN: return "listen";
        case MSG_TYPE_ABORT: return "abort";
        case MSG_TYPE_STT: return "stt";
        case MSG_TYPE_LLM: return "llm";
        case MSG_TYPE_TTS: return "tts";
        case MSG_TYPE_MCP: return "mcp";
        case MSG_TYPE_SYSTEM: return "system";
        case MSG_TYPE_CUSTOM: return "custom";
        default: return "unknown";
    }
}

message_type_t string_to_message_type(const char* type_str) {
    if (!type_str) return MSG_TYPE_UNKNOWN;

    if (strcmp(type_str, "hello") == 0) return MSG_TYPE_HELLO;
    if (strcmp(type_str, "listen") == 0) return MSG_TYPE_LISTEN;
    if (strcmp(type_str, "abort") == 0) return MSG_TYPE_ABORT;
    if (strcmp(type_str, "stt") == 0) return MSG_TYPE_STT;
    if (strcmp(type_str, "llm") == 0) return MSG_TYPE_LLM;
    if (strcmp(type_str, "tts") == 0) return MSG_TYPE_TTS;
    if (strcmp(type_str, "mcp") == 0) return MSG_TYPE_MCP;
    if (strcmp(type_str, "system") == 0) return MSG_TYPE_SYSTEM;
    if (strcmp(type_str, "custom") == 0) return MSG_TYPE_CUSTOM;

    return MSG_TYPE_UNKNOWN;
}

cJSON* create_audio_params_object(const audio_params_t* params) {
    if (!params) return NULL;

    cJSON* obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "format", params->format);
    cJSON_AddNumberToObject(obj, "sample_rate", params->sample_rate);
    cJSON_AddNumberToObject(obj, "channels", params->channels);
    cJSON_AddNumberToObject(obj, "frame_duration", params->frame_duration);

    return obj;
}

bool parse_audio_params_object(cJSON* json, audio_params_t* params) {
    if (!json || !params) return false;

    memset(params, 0, sizeof(audio_params_t));

    cJSON* format_item = cJSON_GetObjectItem(json, "format");
    if (cJSON_IsString(format_item)) {
        strncpy(params->format, format_item->valuestring, sizeof(params->format) - 1);
    }

    cJSON* sample_rate_item = cJSON_GetObjectItem(json, "sample_rate");
    if (cJSON_IsNumber(sample_rate_item)) {
        params->sample_rate = sample_rate_item->valueint;
    }

    cJSON* channels_item = cJSON_GetObjectItem(json, "channels");
    if (cJSON_IsNumber(channels_item)) {
        params->channels = channels_item->valueint;
    }

    cJSON* frame_duration_item = cJSON_GetObjectItem(json, "frame_duration");
    if (cJSON_IsNumber(frame_duration_item)) {
        params->frame_duration = frame_duration_item->valueint;
    }

    return true;
}

cJSON* create_features_object(const features_t* features) {
    if (!features) return NULL;

    cJSON* obj = cJSON_CreateObject();
    cJSON_AddBoolToObject(obj, "mcp", features->mcp);
    // 可以添加其他特性字段

    return obj;
}

bool validate_message_format(cJSON* json) {
    if (!json) return false;

    // 检查必需字段：type
    cJSON* type_item = cJSON_GetObjectItem(json, "type");
    if (!cJSON_IsString(type_item)) {
        return false;
    }

    // 根据类型检查其他必需字段
    const char* type_str = type_item->valuestring;
    message_type_t type = string_to_message_type(type_str);

    switch (type) {
        case MSG_TYPE_LISTEN: {
            cJSON* state_item = cJSON_GetObjectItem(json, "state");
            if (!cJSON_IsString(state_item)) return false;
            break;
        }

        case MSG_TYPE_TTS: {
            cJSON* state_item = cJSON_GetObjectItem(json, "state");
            if (!cJSON_IsString(state_item)) return false;
            break;
        }

        case MSG_TYPE_SYSTEM: {
            cJSON* command_item = cJSON_GetObjectItem(json, "command");
            if (!cJSON_IsString(command_item)) return false;
            break;
        }

        default:
            break;
    }

    return true;
}



//
//// 示例：创建和发送 Hello 消息
//void example_create_hello() {
//    features_t features = { .mcp = true };
//    audio_params_t audio_params = {
//        .format = "opus",
//        .sample_rate = 16000,
//        .channels = 1,
//        .frame_duration = 60
//    };
//
//    cJSON* hello_msg = create_hello_message(1, &features, "websocket", &audio_params);
//    char* json_str = cJSON_Print(hello_msg);
//
//    printf("Created Hello message: %s\n", json_str);
//
//    // 发送到 WebSocket
//    websocket_send_text(json_str);
//
//    free(json_str);
//    cJSON_Delete(hello_msg);
//}
//
//// 示例：解析收到的消息
//void example_parse_message(const char* received_json) {
//    websocket_message_t message;
//
//    if (parse_websocket_message(received_json, &message)) {
//        printf("Received message type: %s\n", message_type_to_string(message.type));
//
//        switch (message.type) {
//            case MSG_TYPE_STT:
//                printf("STT text: %s\n", message.data.stt.text);
//                break;
//
//            case MSG_TYPE_TTS:
//                if (message.data.tts.state == TTS_STATE_SENTENCE_START) {
//                    printf("TTS sentence: %s\n", message.data.tts.text);
//                }
//                break;
//
//            default:
//                break;
//        }
//
//        free_websocket_message(&message);
//    }
//}
//
//// 示例：创建 MCP 消息
//void example_create_mcp() {
//    // 创建 JSON-RPC 2.0 调用
//    cJSON* rpc_call = cJSON_CreateObject();
//    cJSON_AddStringToObject(rpc_call, "jsonrpc", "2.0");
//    cJSON_AddStringToObject(rpc_call, "method", "tools/call");
//
//    cJSON* params = cJSON_CreateObject();
//    cJSON_AddStringToObject(params, "name", "self.light.set_rgb");
//
//    cJSON* arguments = cJSON_CreateObject();
//    cJSON_AddNumberToObject(arguments, "r", 255);
//    cJSON_AddNumberToObject(arguments, "g", 0);
//    cJSON_AddNumberToObject(arguments, "b", 0);
//    cJSON_AddItemToObject(params, "arguments", arguments);
//
//    cJSON_AddItemToObject(rpc_call, "params", params);
//    cJSON_AddNumberToObject(rpc_call, "id", 1);
//
//    // 创建 MCP 消息
//    cJSON* mcp_msg = create_mcp_message("session_123", rpc_call);
//    char* json_str = cJSON_Print(mcp_msg);
//
//    printf("MCP message: %s\n", json_str);
//
//    free(json_str);
//    cJSON_Delete(mcp_msg);
//}

