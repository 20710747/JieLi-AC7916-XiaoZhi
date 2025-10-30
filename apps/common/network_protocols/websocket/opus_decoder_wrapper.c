#include "opus_decoder_wrapper.h"
#include "system/includes.h"
#include "robot_debug.h"
#include "os/os_api.h"

/*
 * Opus解码器包装实现
 * 
 * 注意：此实现需要libopus库支持。有两种集成方式：
 * 
 * 方式1：使用标准libopus库
 * - 需要添加libopus的头文件和库文件到项目中
 * - 在Makefile中链接libopus.a
 * - 取消注释下面的USE_LIBOPUS宏定义
 * 
 * 方式2：使用SDK内置的opus解码器（推荐）
 * - AC7916 SDK已包含opus解码器库
 * - 需要通过audio_server接口使用
 * - 目前此实现提供了基础框架
 */

// 如果项目中已集成libopus库，取消注释以下行
// #define USE_LIBOPUS

#ifdef USE_LIBOPUS
// 使用标准libopus库
#include "opus.h"

struct opus_decoder_wrapper {
    OpusDecoder *decoder;
    int sample_rate;
    int channels;
};

opus_decoder_handle_t opus_decoder_wrapper_init(int sample_rate, int channels)
{
    struct opus_decoder_wrapper *wrapper = (struct opus_decoder_wrapper *)malloc(sizeof(struct opus_decoder_wrapper));
    if (wrapper == NULL) {
        DBG_PRINTF("Failed to allocate opus decoder wrapper\n");
        return NULL;
    }

    int error;
    wrapper->decoder = opus_decoder_create(sample_rate, channels, &error);
    if (error != OPUS_OK || wrapper->decoder == NULL) {
        DBG_PRINTF("Failed to create opus decoder: %d\n", error);
        free(wrapper);
        return NULL;
    }

    wrapper->sample_rate = sample_rate;
    wrapper->channels = channels;
    
    DBG_PRINTF("Opus decoder initialized: sr=%d, ch=%d\n", sample_rate, channels);
    return (opus_decoder_handle_t)wrapper;
}

int opus_decoder_wrapper_decode(opus_decoder_handle_t handle, 
                                const u8 *opus_data, 
                                int opus_len, 
                                s16 *pcm_out, 
                                int max_samples)
{
    if (handle == NULL || opus_data == NULL || pcm_out == NULL) {
        return -1;
    }

    struct opus_decoder_wrapper *wrapper = (struct opus_decoder_wrapper *)handle;
    
    // 解码opus数据
    int decoded_samples = opus_decode(wrapper->decoder, opus_data, opus_len, 
                                     pcm_out, max_samples, 0);
    
    if (decoded_samples < 0) {
        DBG_PRINTF("Opus decode error: %d\n", decoded_samples);
        return -1;
    }

    return decoded_samples;
}

void opus_decoder_wrapper_destroy(opus_decoder_handle_t handle)
{
    if (handle != NULL) {
        struct opus_decoder_wrapper *wrapper = (struct opus_decoder_wrapper *)handle;
        if (wrapper->decoder) {
            opus_decoder_destroy(wrapper->decoder);
        }
        free(wrapper);
        DBG_PRINTF("Opus decoder destroyed\n");
    }
}

#else
// 简化实现：提供框架，需要用户集成libopus或使用SDK的opus解码器

struct opus_decoder_wrapper {
    int sample_rate;
    int channels;
    int frame_size;
};

opus_decoder_handle_t opus_decoder_wrapper_init(int sample_rate, int channels)
{
    struct opus_decoder_wrapper *wrapper = (struct opus_decoder_wrapper *)malloc(sizeof(struct opus_decoder_wrapper));
    if (wrapper == NULL) {
        DBG_PRINTF("Failed to allocate opus decoder wrapper\n");
        return NULL;
    }

    wrapper->sample_rate = sample_rate;
    wrapper->channels = channels;
    wrapper->frame_size = sample_rate * 60 / 1000;  // 60ms帧
    
    DBG_PRINTF("Opus decoder wrapper initialized: sr=%d, ch=%d\n", sample_rate, channels);
    DBG_PRINTF("Warning: Using placeholder decoder. Please integrate libopus library!\n");
    DBG_PRINTF("To integrate libopus:\n");
    DBG_PRINTF("  1. Add opus.h to include path\n");
    DBG_PRINTF("  2. Add libopus.a to link libraries\n");
    DBG_PRINTF("  3. Define USE_LIBOPUS in opus_decoder_wrapper.c\n");
    
    return (opus_decoder_handle_t)wrapper;
}

int opus_decoder_wrapper_decode(opus_decoder_handle_t handle, 
                                const u8 *opus_data, 
                                int opus_len, 
                                s16 *pcm_out, 
                                int max_samples)
{
    if (handle == NULL || opus_data == NULL || pcm_out == NULL) {
        return -1;
    }

    struct opus_decoder_wrapper *wrapper = (struct opus_decoder_wrapper *)handle;
    
    // 占位实现：需要集成真正的opus解码器
    DBG_PRINTF("Warning: Opus decode placeholder called, len=%d\n", opus_len);
    DBG_PRINTF("Please integrate libopus library for actual decoding\n");
    
    // 返回预期的采样点数（用于测试）
    // 实际应用中需要真正解码opus数据
    return wrapper->frame_size;
}

void opus_decoder_wrapper_destroy(opus_decoder_handle_t handle)
{
    if (handle != NULL) {
        free(handle);
        DBG_PRINTF("Opus decoder wrapper destroyed\n");
    }
}

#endif // USE_LIBOPUS

