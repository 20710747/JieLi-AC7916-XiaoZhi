# 如何使用 awtk-widget-qr 二维码控件

## 一、设置需要转成二维码的字符串
qr 控件 `value` 属性内容就是转成二维码的字符串内容。

在界面描述文件设置：
```xml
<window theme="main" name="main">
  <qr name="qr" x="c" y="m" w="50%" h="50%" value="https://github.com/zlgopen/awtk"/>
</window>
```

或在代码中设置：
```c
widget_t* win = window_open("main");
widget_t* qr = widget_lookup(win, "qr", TRUE);
qr_set_value(qr, "https://github.com/zlgopen/awtk");
```

按照上述例子设置完成后，用手机扫该二维码就会打开[ https://github.com/zlgopen/awtk ](https://github.com/zlgopen/awtk)网址。

## 二、设置二维码中的图片
添加图片资源到项目中：

![](../design/default/images/x1/message.png)

在样式文件中设置 qr 控件样式在对应状态设置 `bg_image` 属性：
```xml
<qr>
  <style name="default">
    <normal fg_color="black" bg_color="white" bg_image="message"/>

    <!-- disable 状态用于模拟 被扫描 状态 -->
    <!-- 不设置 bg_image 属性，则该状态二维码中间没有图片 -->
    <disable fg_color="gray" bg_color="#f0f0f0"/>
  </style>
</qr>
```
从上述例子得到的效果为：

正常状态：

![](images/normal_style.png)

被扫描状态：

![](images/scanned_style.png)


### **注意**：图片大小不可以大于二维码的 `45%` ，否则图片将会被裁剪：

图片资源：

![](../design/default/images/x3/message.png)

得到的二维码：

![](images/clip_image.png)
