# video_image 控件

在一般的嵌入式板子上面，解析 JPG 文件或者解析 PNG 文件速度是很慢的，特别在播放高分辨率序列帧的时候尤为明显，此时可以使用 video_image 控件来播放这些序列帧。

video_image 控件采用帧间差异的图像算法，把序列帧压缩为一个自定义的视频文件，这种方法的原理本质上就是空间换时间的策略，虽然整体视频文件的大小会比 JPG 文件（PNG 文件）组成的序列帧要大，但是会比位图文件小很多。

## 一，video_image 控件用法

​	video_image 控件的用法其实很简单，例如，在界面UI中创建一个 video_image 控件，并让该控件播放名字为 `video_12` 的视频文件，代码如下：

``` xml
<window anim_hint="htranslate" style:normal:bg_color="#00000000" >
  <video_image x="0" y="0" w="800" h="480" video_name="video_12" />
</window>
```

​	video_image 控件的属性：

| 属性          | 作用                                                                  |
| ------------- | --------------------------------------------------------------------- |
| video_name    | 需要播放的视频文件名称（该文件需要放在 design/default/data 文件夹中） |
| auto_play     | 是否自动播放（默认为否）                                              |
| delay_play    | 延迟播放时间（默认为0）                                               |
| loop          | 是否循环播放（默认为否）                                              |
| need_replay   | 是否优化重播效率（设置loop为true时该属性默认为true）                  |
| playback_rate | 播放速度（默认为1）                                                   |

在使用上，video_image 控件只需要设置 video_name 属性就可以播放序列帧了，并且播放完成后，会分发 EVT_ANIM_END 事件，可以通过 widget_on 函数注册对应事件的回调函数。

## 二，运行例子

### 准备

1. 获取 awtk 并编译

```
git clone https://github.com/zlgopen/awtk.git
cd awtk; scons; cd -
```

### 运行

1. 生成示例代码的资源

```
python scripts/update_res.py all
```

> 也可以使用 Designer 打开项目，之后点击 “打包” 按钮进行生成；
> 如果资源发生修改，则需要重新生成资源。

2. 编译

```
scons
```

3. 运行
```
./bin/demo
```

## 三，将序列帧图片压缩成视频文件

video_image 控件只负责播放压缩好的视频文件，通过编译生成的 `bin/diff_image_to_video_gen` 工具可以将序列帧压缩成 video_image 控件播放的视频文件，该工具可以在 window 和 linux 上面运行，用法如下：

```bash
./bin/diff_image_to_video_gen [序列帧文件夹路径] [序列帧文件名格式] [视频文件保存路径] [视频文件颜色格式] [帧间延迟时间] [line_length模式(选填)]
```

| 参数                  | 说明                                                       | 示例                                       |
| --------------------- | ---------------------------------------------------------- | ------------------------------------------ |
| 序列帧文件夹路径      |                                                            | design\default\images\video\video_12       |
| 序列帧文件名格式      |                                                            | 序列帧文件名为s01.jpg，则文件名格式为 s%2d |
| 视频文件保存路径      |                                                            | design\default\data\video_12               |
| 视频文件颜色格式      | 可选项：RGBA8888、RGB888、RGB565、BGRA8888、BGR888、BGR565 | RGBA8888                                   |
| 帧间延迟时间          | 每一帧的时间间隔，单位为毫秒                               | 80                                         |
| line_length模式(选填) | 用于解决解压贴图时的 line_length 的对齐问题                | 一般情况下无需填写，缺省为贴图的 w * bpp   |

例如，此处将 `design\default\images\video\video_12` 中的序列帧压缩成视频文件，并保存到 `design\default\data\video_12`，命令如下：

```bash
./bin/diff_image_to_video_gen design/default/images/video/video_12 s%02d design/default/data/video_12 RGBA8888 80
```

**需要注意的是，压缩生成的视频文件需要放在 design/default/data 文件夹中。**

> line_length模式主要用在某些特殊的硬件平台，这些平台为了提高内存读写速度，要求贴图的 line_length 必须按指定规则对齐。
> line_length模式参数的写法：line_length_model_t + ":" + 数据。
> 1. 例如 line_length 长度为贴图宽度 * 16 字节对齐，则参数填写："0:16"。
> 2. 例如 line_length 长度为 2 字节对齐，则参数填写："1:2"。
> 3. 例如 line_length 长度一定是 1024，则参数填写："2:1024"。

## 四，将GIF图片拆解成序列帧图片

在 AWTK 中，显示 GIF 图片的过程是先将 GIF 解析成一帧帧的位图，再将它们拼接起来加载到内存中，比如一个分辨率为 480 * 480 的 GIF 图片，其中包含 120 帧图像，需要将其解析为 16 位色的 bitmap 显示到 LCD 上，那么就至少需要 480 * 480 * 120 * 2 大小的内存，约为 52.7 M。

这么大的内存需求量，一般的嵌入式开发板显然是无法提供的，这个时候就可以借助编译出来的 `bin/gif_to_frame_gen` 工具将 GIF 图片转成一张张的位图序列帧，然后再使用 `bin/diff_image_to_video_gen` 工具压缩成 video_image 控件播放的视频文件。

`bin/gif_to_frame_gen` 工具的用法如下：

```bash
./bin/gif_to_frame_gen [GIF图片路径] [保存序列帧的文件夹路径]
```

| 参数                   | 示例                                   |
| ---------------------- | -------------------------------------- |
| GIF图片路径            | design\default\images\video\gif_35.gif |
| 保存序列帧的文件夹路径 | design/default/images/video/gif_35     |

例如，此处将 `design\default\images\video\gif_35.gif` 拆解成对应的序列帧，命令如下：

```bash
./bin/gif_to_frame_gen design/default/images/video/gif_35.gif design/default/images/video/gif_35
```

执行成功后，可以在 `design/default/images/video/gif_35` 目录下看到对应的序列帧文件，再通过 `bin/diff_image_to_video_gen` 工具将这些序列帧压缩成视频文件，就可以用 video_image 控件播放了。

## 文档

[优化 video_image 控件效率](./docs/how_to_optimize_efficiency.md)
[AWTK 自定义控件规范](https://github.com/zlgopen/awtk/blob/master/docs/custom_widget_rules.md)
