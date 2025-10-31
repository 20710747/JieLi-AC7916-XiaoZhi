# AC79 SDK Opus音频编解码函数详细分析

## 📚 目录
1. [Opus解码器函数](#opus解码器函数)
2. [PCM音频播放函数](#pcm音频播放函数)
3. [Audio Server服务器函数](#audio-server服务器函数)
4. [标准libopus库函数](#标准libopus库函数)
5. [完整调用流程](#完整调用流程)
6. [集成方案](#集成方案)

---

## 1️⃣ Opus解码器函数

### 1.1 自定义包装器函数（项目实现）

位置：`apps/common/network_protocols/websocket/opus_decoder_wrapper.c`

#### 📌 `opus_decoder_wrapper_init()`
**函数原型**：
```c
opus_decoder_handle_t opus_decoder_wrapper_init(int sample_rate, int channels);
```

**功能**：初始化Opus解码器

**参数**：
- `sample_rate`: 采样率 (支持8000/16000/24000/48000 Hz)
- `channels`: 声道数 (1=单声道, 2=双声道)

**返回值**：
- 成功：解码器句柄（void*类型）
- 失败：NULL

**内部实现**：
```c
// 方式1: 使用标准libopus库（需定义USE_LIBOPUS）
wrapper->decoder = opus_decoder_create(sample_rate, channels, &error);

// 方式2: 占位实现（需用户集成真正的opus库）
wrapper->frame_size = sample_rate * 60 / 1000;  // 60ms帧
```

**使用示例**：
```c
// 初始化16kHz单声道解码器
opus_decoder_handle_t decoder = opus_decoder_wrapper_init(16000, 1);
if (decoder == NULL) {
    printf("Failed to init opus decoder\n");
}
```

---

#### 📌 `opus_decoder_wrapper_decode()`
**函数原型**：
```c
int opus_decoder_wrapper_decode(
    opus_decoder_handle_t handle,  // 解码器句柄
    const u8 *opus_data,            // Opus编码数据
    int opus_len,                   // 数据长度
    s16 *pcm_out,                   // PCM输出缓冲区
    int max_samples                 // 最大输出采样点数
);
```

**功能**：解码Opus数据为PCM音频

**参数**：
- `handle`: 解码器句柄（由init函数返回）
- `opus_data`: Opus编码的音频数据（二进制格式）
- `opus_len`: Opus数据长度（字节）
- `pcm_out`: PCM输出缓冲区（int16格式）
- `max_samples`: 最大输出采样点数（通常为帧大小）

**返回值**：
- 成功：实际解码的采样点数
- 失败：负数

**内部实现**：
```c
// 使用标准libopus库
int decoded_samples = opus_decode(
    wrapper->decoder,   // libopus解码器
    opus_data,          // 输入数据
    opus_len,           // 输入长度
    pcm_out,            // 输出缓冲区
    max_samples,        // 最大采样点
    0                   // 不使用FEC（前向纠错）
);
```

**使用示例**：
```c
s16 pcm_buffer[960];  // 16kHz * 60ms = 960采样点
int samples = opus_decoder_wrapper_decode(
    decoder,
    opus_frame_data,   // WebSocket接收的opus数据
    opus_frame_len,    // 数据长度
    pcm_buffer,        // 输出到此缓冲区
    960                // 最大960采样点
);

if (samples > 0) {
    printf("Decoded %d samples\n", samples);
    // PCM数据字节数 = samples * 2字节/采样点 * 声道数
    int pcm_bytes = samples * 2 * 1;
}
```

---

#### 📌 `opus_decoder_wrapper_destroy()`
**函数原型**：
```c
void opus_decoder_wrapper_destroy(opus_decoder_handle_t handle);
```

**功能**：释放Opus解码器资源

**参数**：
- `handle`: 解码器句柄

**使用示例**：
```c
opus_decoder_wrapper_destroy(decoder);
decoder = NULL;
```

---

## 2️⃣ PCM音频播放函数

### 2.1 PCM播放器管理函数

位置：`apps/common/audio_music/pcm_play_api.c`

#### 📌 `audio_pcm_play_open()`
**函数原型**：
```c
void *audio_pcm_play_open(
    int sample_rate,      // 采样率
    u32 frame_size,       // 帧缓冲大小
    u32 drop_points,      // 丢弃点数
    u8 channel,           // 声道数
    u8 volume,            // 音量 0-100
    u8 block              // 阻塞模式 0=非阻塞 1=阻塞
);
```

**功能**：打开PCM音频播放器

**参数**：
- `sample_rate`: 采样率 (Hz)，如16000
- `frame_size`: 帧缓冲大小（字节），建议为 PCM_BUFFER_SIZE * 4
- `drop_points`: 丢弃点数（用于音频同步）
- `channel`: 声道数 (1=单声道, 2=双声道)
- `volume`: 音量 (0-100)
- `block`: 阻塞模式 (0=非阻塞推荐, 1=阻塞)

**返回值**：
- 成功：播放器句柄
- 失败：NULL

**内部流程**：
1. 分配 `struct audio_pcm_play_t` 结构体
2. 创建循环缓冲区 `cbuffer_t`
3. 打开audio_server的解码服务
4. 配置解码器参数（PCM格式、DAC输出）
5. 创建信号量用于同步

**使用示例**：
```c
void *pcm_player = audio_pcm_play_open(
    16000,              // 16kHz采样率
    1920 * 4,          // 缓冲区大小
    0,                 // 不丢弃点
    1,                 // 单声道
    80,                // 音量80
    0                  // 非阻塞模式
);

if (pcm_player == NULL) {
    printf("Failed to open PCM player\n");
}
```

---

#### 📌 `audio_pcm_play_start()`
**函数原型**：
```c
int audio_pcm_play_start(void *priv);
```

**功能**：启动PCM播放器

**参数**：
- `priv`: 播放器句柄（由open函数返回）

**返回值**：
- 0：成功
- -1：失败

**内部实现**：
```c
req.dec.cmd = AUDIO_DEC_START;
server_request(hdl->dec_server, AUDIO_REQ_DEC, &req);
```

**使用示例**：
```c
if (audio_pcm_play_start(pcm_player) != 0) {
    printf("Failed to start PCM player\n");
}
```

---

#### 📌 `audio_pcm_play_data_write()`
**函数原型**：
```c
int audio_pcm_play_data_write(
    void *priv,        // 播放器句柄
    void *data,        // PCM数据
    u32 size           // 数据大小（字节）
);
```

**功能**：写入PCM数据到播放器

**参数**：
- `priv`: 播放器句柄
- `data`: PCM数据缓冲区（int16格式）
- `size`: 数据大小（字节）

**返回值**：
- 成功：写入的字节数
- 失败：-1

**内部流程**：
1. 将PCM数据写入循环缓冲区 `cbuf_write()`
2. 发送信号量通知解码线程
3. 解码线程从缓冲区读取数据
4. 通过audio_server输出到DAC

**使用示例**：
```c
s16 pcm_data[960];  // 解码后的PCM数据
int pcm_size = 960 * 2;  // 1920字节

int ret = audio_pcm_play_data_write(pcm_player, pcm_data, pcm_size);
if (ret < 0) {
    printf("Failed to write PCM data\n");
}
```

---

#### 📌 `audio_pcm_play_stop()`
**函数原型**：
```c
int audio_pcm_play_stop(void *priv);
```

**功能**：停止并释放PCM播放器

**参数**：
- `priv`: 播放器句柄

**返回值**：0

**内部流程**：
1. 停止解码器
2. 关闭audio_server
3. 释放缓冲区
4. 删除信号量
5. 释放结构体内存

**使用示例**：
```c
audio_pcm_play_stop(pcm_player);
pcm_player = NULL;
```

---

#### 📌 其他辅助函数

**`audio_pcm_play_pause()`**
```c
int audio_pcm_play_pause(void *priv, int clear_cache);
```
- 暂停播放器
- `clear_cache=1` 时清空缓冲区

**`audio_pcm_play_set_volume()`**
```c
int audio_pcm_play_set_volume(void *priv, u8 volume);
```
- 动态设置音量（0-100）

---

## 3️⃣ Audio Server服务器函数

### 3.1 核心服务接口

位置：`include_lib/server/audio_server.h`

#### 📌 `server_open()`
```c
void *server_open(const char *name, const char *mode);
```
**功能**：打开audio服务器

**使用示例**：
```c
void *audio_server = server_open("audio_server", "dec");
```

---

#### 📌 `server_request()`
```c
int server_request(void *server, int req_type, union audio_req *req);
```
**功能**：向服务器发送请求

**常用请求类型**：
- `AUDIO_REQ_DEC`: 解码器请求
- `AUDIO_REQ_ENC`: 编码器请求

**常用命令**：
- `AUDIO_DEC_OPEN`: 打开解码器
- `AUDIO_DEC_START`: 启动解码
- `AUDIO_DEC_STOP`: 停止解码
- `AUDIO_DEC_PAUSE`: 暂停解码
- `AUDIO_DEC_SET_VOLUME`: 设置音量

**使用示例**：
```c
union audio_req req = {0};
req.dec.cmd = AUDIO_DEC_OPEN;
req.dec.volume = 80;
req.dec.sample_rate = 16000;
req.dec.channel = 1;
req.dec.dec_type = "pcm";  // 解码类型
req.dec.sample_source = "dac";  // 输出到DAC

server_request(audio_server, AUDIO_REQ_DEC, &req);
```

---

#### 📌 `server_close()`
```c
int server_close(void *server);
```
**功能**：关闭服务器

---

### 3.2 音频格式定义

位置：`include_lib/server/audio_dev.h`

**支持的音频格式**：
```c
#define AUDIO_FMT_PCM          0x01  // PCM格式
#define AUDIO_FMT_SPEEX        0x02  // Speex编解码
#define AUDIO_FMT_AMR          0x03  // AMR编解码
#define AUDIO_FMT_AAC          0x04  // AAC编解码
#define AUDIO_FMT_OPUS         0x05  // Opus编解码 ⭐
#define AUDIO_FMT_CVSD         0x06  // CVSD编解码
#define AUDIO_FMT_MSBC         0x07  // MSBC编解码
#define AUDIO_FMT_ADPCM        0x08  // ADPCM编解码
#define AUDIO_FMT_SBC          0x09  // SBC编解码
#define AUDIO_FMT_MP3          0x0b  // MP3编解码
```

---

## 4️⃣ 标准libopus库函数

### 4.1 解码器函数

当定义 `USE_LIBOPUS` 时使用标准libopus库

#### 📌 `opus_decoder_create()`
**函数原型**：
```c
OpusDecoder *opus_decoder_create(
    opus_int32 Fs,       // 采样率
    int channels,        // 声道数
    int *error           // 错误码输出
);
```

**功能**：创建Opus解码器实例

**采样率支持**：
- 8000 Hz
- 12000 Hz
- 16000 Hz（常用）
- 24000 Hz
- 48000 Hz

**错误码**：
- `OPUS_OK (0)`: 成功
- `OPUS_BAD_ARG`: 参数错误
- `OPUS_ALLOC_FAIL`: 内存分配失败

---

#### 📌 `opus_decode()`
**函数原型**：
```c
int opus_decode(
    OpusDecoder *st,             // 解码器实例
    const unsigned char *data,   // 输入数据
    opus_int32 len,              // 数据长度
    opus_int16 *pcm,             // 输出PCM
    int frame_size,              // 帧大小
    int decode_fec               // FEC解码标志
);
```

**功能**：解码一帧Opus数据

**参数说明**：
- `decode_fec`: 
  - 0: 正常解码
  - 1: 使用FEC（Forward Error Correction）丢包补偿

**返回值**：
- 成功：解码的采样点数
- 失败：负数错误码

---

#### 📌 `opus_decoder_destroy()`
**函数原型**：
```c
void opus_decoder_destroy(OpusDecoder *st);
```

**功能**：销毁解码器并释放资源

---

### 4.2 编码器函数（扩展）

#### 📌 `opus_encoder_create()`
```c
OpusEncoder *opus_encoder_create(
    opus_int32 Fs,           // 采样率
    int channels,            // 声道数
    int application,         // 应用场景
    int *error               // 错误码
);
```

**应用场景**：
- `OPUS_APPLICATION_VOIP`: VoIP语音（低延迟）
- `OPUS_APPLICATION_AUDIO`: 音乐（高质量）
- `OPUS_APPLICATION_RESTRICTED_LOWDELAY`: 受限低延迟

---

#### 📌 `opus_encode()`
```c
int opus_encode(
    OpusEncoder *st,
    const opus_int16 *pcm,       // 输入PCM
    int frame_size,              // 帧大小
    unsigned char *data,         // 输出数据
    opus_int32 max_data_bytes    // 最大输出字节
);
```

**返回值**：编码后的字节数

---

## 5️⃣ 完整调用流程

### 5.1 Opus解码播放流程

```
┌─────────────────────────────────────────────────────────┐
│              WebSocket接收Opus音频流                    │
└──────────────────────┬──────────────────────────────────┘
                       │
                       ▼
       ┌───────────────────────────────┐
       │  websockets_callback()        │
       │  (buf, len, type=130)         │
       └───────────────┬───────────────┘
                       │
                       ▼
       ┌───────────────────────────────┐
       │  判断是否已初始化播放器       │
       └───────────────┬───────────────┘
                       │
                       ▼
       ┌───────────────────────────────┐
       │  audio_player_init()          │
       │  ├─ opus_decoder_wrapper_init() ──→ 初始化Opus解码器
       │  ├─ audio_pcm_play_open()      ──→ 打开PCM播放器
       │  └─ audio_pcm_play_start()     ──→ 启动播放器
       └───────────────┬───────────────┘
                       │
                       ▼
       ┌───────────────────────────────┐
       │  opus_decoder_wrapper_decode()│
       │  ├─ 输入: Opus数据            │
       │  └─ 输出: PCM数据(s16)        │
       └───────────────┬───────────────┘
                       │
                       ▼
       ┌───────────────────────────────┐
       │  audio_play_pcm_data()        │
       │  └─ audio_pcm_play_data_write() ──→ 写入PCM数据
       └───────────────┬───────────────┘
                       │
                       ▼
       ┌───────────────────────────────┐
       │  cbuf_write()                 │
       │  (写入循环缓冲区)             │
       └───────────────┬───────────────┘
                       │
                       ▼
       ┌───────────────────────────────┐
       │  audio_server解码线程         │
       │  ├─ cbuf_read()               │
       │  └─ 从缓冲区读取PCM数据       │
       └───────────────┬───────────────┘
                       │
                       ▼
       ┌───────────────────────────────┐
       │  DAC硬件输出音频              │
       └───────────────────────────────┘
```

### 5.2 关键数据流

**Opus音频参数（WebSocket示例）**：
```
采样率：16000 Hz
声道数：1 (单声道)
帧时长：60 ms
每帧采样点：960 (16000 * 0.06)
每帧PCM大小：1920 字节 (960 * 2 * 1)
```

**缓冲区设置**：
```c
#define OPUS_SAMPLE_RATE    16000
#define OPUS_CHANNELS       1
#define OPUS_FRAME_DURATION 60
#define OPUS_FRAME_SIZE     (OPUS_SAMPLE_RATE * OPUS_FRAME_DURATION / 1000)  // 960
#define PCM_BUFFER_SIZE     (OPUS_FRAME_SIZE * 2 * OPUS_CHANNELS)  // 1920字节
```

---

## 6️⃣ 集成方案

### 方案1：使用标准libopus库（推荐）

**步骤1：获取libopus库**
```bash
# 下载opus源码
git clone https://github.com/xiph/opus.git
cd opus

# 为AC79平台交叉编译
./configure --host=arm-linux --enable-fixed-point
make
```

**步骤2：集成到项目**
```
1. 复制 include/opus.h 到项目include目录
2. 复制 .libs/libopus.a 到项目lib目录
```

**步骤3：启用libopus**
```c
// 在 opus_decoder_wrapper.c 中取消注释
#define USE_LIBOPUS
```

**步骤4：修改Makefile**
```makefile
# 添加源文件
objs += \
    $(src)/apps/common/network_protocols/websocket/opus_decoder_wrapper.o

# 添加头文件路径
INCLUDES += \
    -I$(src)/include_lib/opus

# 链接libopus库
LIBS += $(src)/lib/libopus.a
# 或者
LIBS += -lopus
```

---

### 方案2：使用SDK内置opus库

**步骤1：链接SDK的opus库**
```makefile
# 在Makefile中添加
LIBS += $(src)/cpu/wl82/liba/lib_opus_dec.a
LIBS += $(src)/cpu/wl82/liba/lib_opus_enc.a
```

**步骤2：通过audio_server使用**
```c
// 配置audio_req时指定opus格式
union audio_req req = {0};
req.dec.dec_type = "opus";  // 指定opus解码器
req.dec.format = AUDIO_FMT_OPUS;
req.dec.sample_rate = 16000;
req.dec.channel = 1;

server_request(audio_server, AUDIO_REQ_DEC, &req);
```

**注意事项**：
1. SDK内置库可能需要特定的音频配置
2. 需要启用 `CONFIG_OPUS_DEC_ENABLE` 宏
3. 查看 `apps/common/audio_music/audio_config.c`:
```c
#ifdef CONFIG_OPUS_DEC_ENABLE
const int silk_fsN_enable = 1;  // 支持8-12k采样率
const int silk_fsW_enable = 1;  // 支持16-24k采样率
#endif
```

---

## 7️⃣ 函数调用示例代码

### 完整示例：WebSocket接收并播放Opus音频

```c
#include "opus_decoder_wrapper.h"
#include "pcm_play_api.h"

// 全局变量
static opus_decoder_handle_t g_opus_decoder = NULL;
static void *g_pcm_player = NULL;

// 初始化
int init_opus_player(void) {
    // 1. 初始化Opus解码器
    g_opus_decoder = opus_decoder_wrapper_init(16000, 1);
    if (g_opus_decoder == NULL) {
        return -1;
    }
    
    // 2. 打开PCM播放器
    g_pcm_player = audio_pcm_play_open(
        16000,      // 采样率
        1920 * 4,   // 缓冲区
        0,          // 不丢点
        1,          // 单声道
        80,         // 音量
        0           // 非阻塞
    );
    
    if (g_pcm_player == NULL) {
        opus_decoder_wrapper_destroy(g_opus_decoder);
        return -1;
    }
    
    // 3. 启动播放器
    if (audio_pcm_play_start(g_pcm_player) != 0) {
        audio_pcm_play_stop(g_pcm_player);
        opus_decoder_wrapper_destroy(g_opus_decoder);
        return -1;
    }
    
    return 0;
}

// WebSocket回调
void on_opus_data_received(u8 *opus_data, int opus_len) {
    // 分配PCM缓冲区
    s16 pcm_buffer[960];  // 16kHz * 60ms
    
    // 解码Opus数据
    int samples = opus_decoder_wrapper_decode(
        g_opus_decoder,
        opus_data,
        opus_len,
        pcm_buffer,
        960
    );
    
    if (samples > 0) {
        // 计算PCM数据大小
        int pcm_size = samples * 2 * 1;  // samples * sizeof(s16) * channels
        
        // 写入播放器
        audio_pcm_play_data_write(g_pcm_player, pcm_buffer, pcm_size);
    }
}

// 清理
void cleanup_opus_player(void) {
    if (g_pcm_player) {
        audio_pcm_play_stop(g_pcm_player);
        g_pcm_player = NULL;
    }
    
    if (g_opus_decoder) {
        opus_decoder_wrapper_destroy(g_opus_decoder);
        g_opus_decoder = NULL;
    }
}
```

---

## 8️⃣ 常见问题和解决方案

### Q1: 编译错误：opus_* 未定义
**解决**：
1. 确认已定义 `USE_LIBOPUS` 宏
2. 确认已添加 `opus.h` 头文件路径
3. 确认已链接 `libopus.a` 库文件

### Q2: 没有声音输出
**检查**：
1. 音量设置是否正确 (0-100)
2. DAC配置是否正确
3. 采样率和声道数是否匹配
4. 使用示波器或逻辑分析仪检查I2S/DAC信号

### Q3: 播放卡顿
**优化**：
1. 增大缓冲区大小 `PCM_BUFFER_SIZE * 4`
2. 使用非阻塞模式
3. 提高任务优先级
4. 检查WebSocket接收速率

### Q4: 编译时找不到lib_opus_dec.a
**解决**：
SDK内置库位于 `cpu/wl82/liba/` 目录下，确认文件存在

---

## 9️⃣ 相关文件清单

### 核心文件
- `apps/common/network_protocols/websocket/opus_decoder_wrapper.h` - Opus解码器接口
- `apps/common/network_protocols/websocket/opus_decoder_wrapper.c` - Opus解码器实现
- `apps/common/network_protocols/websocket/xz_main.c` - WebSocket使用示例
- `apps/common/audio_music/pcm_play_api.h` - PCM播放API头文件
- `apps/common/audio_music/pcm_play_api.c` - PCM播放API实现

### 头文件
- `include_lib/server/audio_server.h` - Audio服务器接口
- `include_lib/server/audio_dev.h` - 音频设备接口

### 库文件
- `cpu/wl82/liba/lib_opus_dec.a` - SDK内置Opus解码库
- `cpu/wl82/liba/lib_opus_enc.a` - SDK内置Opus编码库

### 配置文件
- `apps/common/audio_music/audio_config.c` - 音频配置

---

## 🔟 参考资源

- **Opus官方文档**: https://opus-codec.org/docs/
- **AC79 SDK文档**: https://doc.zh-jieli.com/AC79/
- **libopus GitHub**: https://github.com/xiph/opus
- **RFC 6716**: Opus音频编解码标准

---

**文档版本**: v1.0  
**最后更新**: 2025-10-31  
**适用SDK**: AC79NN_SDK_V1.2.0

