# awtk-csv-file

操作 [CSV 文件](https://en.wikipedia.org/wiki/Comma-separated_values) 库。主要特色：

* 读写文件。
* 遍历数据。
* 支持数据过滤。
* 支持与 MVVM 绑定。

## 准备

1. 获取 awtk 并编译

```
git clone https://github.com/zlgopen/awtk.git
cd awtk; scons; cd -
```

2. 获取 awtk-csv-file 并编译
```
git clone https://github.com/zlgopen/awtk-csv-file.git
cd awtk-csv-file; scons
```

## 运行示例

```
./bin/csv_dump
```

## 示例

```c
static void csv_file_demo(const char* filename) {
  uint32_t r = 0;
  uint32_t c = 0;
  csv_file_t* csv = csv_file_create(filename, ',');

  uint32_t rows = csv_file_get_rows(csv);
  uint32_t cols = csv_file_get_cols(csv);

  if (csv->has_title) {
    r++;
    log_debug("title:%s\n", csv_file_get_title(csv));
  }

  for (; r < rows; r++) {
    log_debug("%d: ", r);
    for (c = 0; c < cols; c++) {
      log_debug("%s, ", csv_file_get(csv, r, c));
    }
    log_debug("\n");
  }

  csv_file_save(csv, "output.csv");
  csv_file_destroy(csv);
}
```
