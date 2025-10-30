# awtk-widget-table-view-mvvm

table view 有点特殊，awtk-mvvm 缺省的方法不支持 table view 的绑定，本项目实现了一个自定义的 binder，让 awtk-mvvm 支持 table view 的绑定。

## 准备

1. 获取 awtk 并编译

```
git clone https://github.com/zlgopen/awtk.git
cd awtk; scons; cd -
```

2. 获取 awtk-mvvm 并编译

```
git clone https://github.com/zlgopen/awtk-mvvm.git
```

参阅 awtk-mvvm 的 README.md 步骤进行编译。

3. 获取 awtk-widget-table-view 并编译

```
git clone https://github.com/zlgopen/awtk-widget-table-view.git
cd awtk-widget-table-view; scons; cd -
```

## 运行

1. 生成示例代码的资源

```
python scripts/update_res.py all
```
> 也可以使用 Designer 打开项目，之后点击 “打包” 按钮进行生成；
> 如果资源发生修改，则需要重新生成资源。

如果 PIL 没有安装，执行上述脚本可能会出现如下错误：
```cmd
Traceback (most recent call last):
...
ModuleNotFoundError: No module named 'PIL'
```
请用 pip 安装：
```cmd
pip install Pillow
```

2. 编译

```
scons
```

3. 运行

* 基本示例

```
./bin/jsdemo
```

* csv 文件查看和编辑

```
./bin/csv_view
```

## 用法

```c
// demos\csv_view\application.c
static view_model_t* csv_view_model_create(navigator_request_t* req) {
  csv_file_t* file = csv_file_create("data/scores.csv", ',');

  if (file != NULL) {
    object_t* obj = csv_file_object_create(file);
    if (obj != NULL) {
      view_model_t* view_model = view_model_array_object_wrapper_create(obj);
      return view_model;
    }
    csv_file_destroy(file);
  }

  return NULL;
}

ret_t application_init() {
  table_view_register();
  table_client_custom_binder_register();

  view_model_factory_register("csv_view", csv_view_model_create);

  return navigator_to("csv_view");
}
```

## 参考

* [awtk-mvvm-c-examples](https://github.com/zlgopen/awtk-mvvm-c-examples)
* [awtk-mvvm](https://github.com/zlgopen/awtk-mvvm)
