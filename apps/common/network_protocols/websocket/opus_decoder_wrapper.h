#ifndef OPUS_DECODER_WRAPPER_H
#define OPUS_DECODER_WRAPPER_H

#include "generic/typedef.h"

// Opus解码器句柄类型
typedef void* opus_decoder_handle_t;

/**
 * @brief 初始化opus解码器
 * @param sample_rate 采样率 (8000, 16000, 24000, 48000)
 * @param channels 声道数 (1或2)
 * @return 解码器句柄，失败返回NULL
 */
opus_decoder_handle_t opus_decoder_wrapper_init(int sample_rate, int channels);

/**
 * @brief 解码opus数据为PCM
 * @param handle 解码器句柄
 * @param opus_data opus编码数据
 * @param opus_len opus数据长度
 * @param pcm_out PCM输出缓冲区 (int16格式)
 * @param max_samples 最大输出采样点数
 * @return 实际解码的采样点数，失败返回负数
 */
int opus_decoder_wrapper_decode(opus_decoder_handle_t handle, 
                                const u8 *opus_data, 
                                int opus_len, 
                                s16 *pcm_out, 
                                int max_samples);

/**
 * @brief 释放opus解码器
 * @param handle 解码器句柄
 */
void opus_decoder_wrapper_destroy(opus_decoder_handle_t handle);

#endif // OPUS_DECODER_WRAPPER_H

