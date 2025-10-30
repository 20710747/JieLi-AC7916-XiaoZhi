# 杰理AC7916 WebSocket Opus音频播放实现说明

## 概述

本实现为杰理AC7916开发板的WebSocket客户端添加了Opus音频流的接收和播放功能。支持实时接收并播放来自服务器的Opus编码音频数据。

## 功能特性

1. **WebSocket数据处理**
   - type=129 (0x81): JSON格式消息处理
   - type=130 (0x82): 二进制Opus音频流处理

2. **Opus音频参数**
   - 采样率: 16000 Hz
   - 声道数: 1 (单声道)
   - 帧时长: 60 ms
   - 每帧采样点: 960 samples

3. **音频播放流程**
   - 接收Opus编码数据
   - 使用Opus解码器解码为PCM数据
   - 通过PCM播放API播放音频

## 文件说明

### 1. xz_main.c
主要的WebSocket客户端实现文件，包含：
- `websockets_callback()`: WebSocket消息回调函数，处理JSON和Opus数据
- `audio_player_init()`: 初始化PCM播放器和Opus解码器
- `audio_player_stop()`: 停止并清理音频播放器
- `audio_play_pcm_data()`: 播放PCM数据

### 2. opus_decoder_wrapper.h
Opus解码器包装接口头文件

### 3. opus_decoder_wrapper.c
Opus解码器包装实现文件，提供两种实现模式：
- USE_LIBOPUS: 使用标准libopus库
- 占位实现: 提供框架，需要用户集成libopus

## 集成libopus库的方法

### 方法1：使用标准libopus库（推荐）

#### 步骤1: 下载libopus库
```bash
# 下载opus源码
git clone https://github.com/xiph/opus.git
cd opus

# 或下载预编译的库文件
wget https://archive.mozilla.org/pub/opus/opus-1.3.1.tar.gz
tar xzf opus-1.3.1.tar.gz
```

#### 步骤2: 编译或获取AC7916平台的libopus
需要为杰理AC7916平台（架构可能是ARM或RISC-V）交叉编译libopus：

```bash
# 假设使用ARM工具链
./configure --host=arm-linux --enable-fixed-point --disable-float-api
make
```

注意：需要根据AC7916实际的CPU架构选择正确的交叉编译工具链。

#### 步骤3: 将文件添加到项目
1. 将 `opus.h` 等头文件复制到项目的 include 目录
2. 将编译好的 `libopus.a` 复制到项目的 lib 目录

#### 步骤4: 修改opus_decoder_wrapper.c
取消注释文件开头的宏定义：
```c
#define USE_LIBOPUS
```

#### 步骤5: 修改Makefile
在项目的Makefile中添加：
```makefile
# 添加libopus的头文件路径
INCLUDES += -I$(SDK_PATH)/include_lib/opus

# 添加libopus库文件
LIBS += -lopus
```

### 方法2：使用AC7916 SDK内置的opus解码器

AC7916 SDK已经包含了opus解码器库（在cpu/wl82/liba/目录下有lib_opus_dec.a）。

#### 使用audio_server的opus解码器（高级方法）

```c
// 创建opus解码请求
union audio_req req = {0};
req.dec.cmd = AUDIO_DEC_OPEN;
req.dec.dec_type = "opus";  // 指定解码类型为opus
req.dec.sample_rate = 16000;
req.dec.channel = 1;
req.dec.volume = 80;
// ... 其他参数设置

// 通过audio_server请求解码
void *dec_server = server_open("audio_server", "dec");
server_request(dec_server, AUDIO_REQ_DEC, &req);
```

注意：此方法需要更复杂的设置，包括虚拟文件系统操作等。

### 方法3：简化测试方案

如果暂时无法集成opus解码库，可以修改服务器端，让服务器直接发送PCM数据：

在 `xz_main.c` 的 `websockets_callback()` 函数中：
```c
else if (type == 130) {
    // 假设服务器发送的是PCM数据而非opus数据
    if (!g_audio_playing) {
        audio_player_init();
    }
    // 直接播放PCM数据
    audio_play_pcm_data(buf, len);
}
```

## 使用说明

### 1. 编译项目
```bash
cd /path/to/jieli-ac7916-xiaozhi
make
```

### 2. 烧录固件
使用杰理的烧录工具将编译好的固件烧录到AC7916开发板

### 3. 运行和调试
- 连接串口查看调试输出
- 确保开发板已连接到WiFi网络
- WebSocket服务器需要运行在配置的地址上

### 4. 调试信息
代码中包含详细的调试输出，可以通过串口监控：
- Opus数据接收信息
- 解码器初始化状态
- PCM播放状态
- 音频数据播放信息

## 测试验证

### 检查点
1. **WebSocket连接**: 查看日志确认WebSocket成功连接
2. **Session ID**: 确认收到服务器的Hello消息并保存了session_id
3. **TTS消息**: 确认收到TTS开始消息时初始化了音频播放器
4. **Opus数据**: 确认收到type=130的二进制数据
5. **解码器**: 确认Opus解码器成功初始化
6. **PCM播放**: 确认PCM数据成功写入播放器

### 常见问题

**Q: 为什么没有声音？**
A: 检查以下几点：
1. Opus解码器是否正确初始化（查看USE_LIBOPUS是否定义）
2. PCM播放器是否成功打开和启动
3. 音量设置是否合适（当前设置为80）
4. DAC输出是否正确配置

**Q: 解码失败怎么办？**
A: 
1. 确认服务器发送的确实是Opus格式数据
2. 确认Opus参数匹配（16000Hz, 1通道, 60ms帧）
3. 如果使用占位实现，需要集成真正的libopus库

**Q: 播放有卡顿？**
A: 
1. 增大PCM缓冲区大小
2. 检查网络延迟
3. 优化解码和播放的处理流程
4. 考虑使用双缓冲机制

## 性能优化建议

1. **内存优化**
   - 使用静态缓冲区代替动态分配
   - 实现缓冲区复用机制

2. **实时性优化**
   - 使用DMA传输PCM数据
   - 提高音频处理线程优先级
   - 减少不必要的调试输出

3. **音质优化**
   - 根据实际情况调整音量
   - 实现音频重采样（如果需要）
   - 添加音频效果处理

## 扩展功能

1. **支持更多音频格式**
   - AAC解码支持
   - MP3解码支持

2. **音频控制功能**
   - 动态音量调节
   - 播放/暂停控制
   - 音频队列管理

3. **错误处理增强**
   - 自动重连机制
   - 解码错误恢复
   - 音频同步校正

## 技术支持

如有问题，请参考：
1. 杰理AC7916 SDK文档
2. AC79xx音频服务使用开发文档.pdf
3. libopus官方文档: https://opus-codec.org/docs/

## 版本历史

- v1.0 (2025-10-30)
  - 初始实现
  - 支持Opus音频流接收和播放
  - 提供libopus集成框架

