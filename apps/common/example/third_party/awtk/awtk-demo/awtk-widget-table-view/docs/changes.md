# 最新动态

2022/04/11
  * 修复table_client控件取消指针按键事件中没有正确复位的问题（感谢雨欣提供补丁）  

2022/04/08
  * 修复table_client控件在子控件的取消指针按键事件返回RET_STOP时会设置滚动条的问题（感谢雨欣提供补丁）  

2021/05/22
  * 添加 table_client_set_on_prepare_row 接口，主要用于 MVVM 绑定。
  * 完善 Demo。

2021/05/22
  * fix table client layout

2021/05/12
  * 修复table\_row创建行时回调函数无效的问题（感谢雨欣提供补丁）

2020/12/16
  * 修复awtk-widget-table-view中widget\_move\_resize函数无效导致行重叠的问题(感谢雨欣提供补丁)。

2020/12/15
  * 修复awtk-widget-table-view在designer中默认xml有误导致行错位的问题(感谢雨欣提供补丁)。

2020/12/05
  * 修复table\_client\_scroll\_to中导致极端情况下，向下拖动不复位的问题。

2020/12/03
  * 修复调整大小是资源的问题(感谢兆坤提供补丁)
  * 调整大小后根据需要创建子控件。
