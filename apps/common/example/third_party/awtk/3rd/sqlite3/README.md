# awtk-sqlite3

## 介绍

[Sqlite](https://www.sqlite.org/index.html) 基于 [AWTK](https://github.com/zlgopen/awtk) 的移植。

[AWTK](https://github.com/zlgopen/awtk) 本身提供了对文件系统、互斥锁、内存等平台相关的 API 的封装，把 sqlite 移植到 AWTK，开发者在嵌入式系统中使用 sqlite 时，不需要自己去移植 sqlite，可以省去不少工作量。

> 在不熟悉 sliqte 的情况下，要将 sqlite 移植到 MCU 平台，并不是件容易的事情。所幸 [RT-Thread 移植的 Sqlite](https://github.com/RT-Thread-packages/SQLite.git) 可以供我们参考，感谢原作者。

## PC 编译
1. 获取 awtk 并编译

```
git clone https://github.com/zlgopen/awtk.git
cd awtk; scons; cd -
```

> PC 版本主要用于功能性测试。

2. 获取 awtk-sqlite3 并编译

```
git clone https://github.com/zlgopen/awtk-sqlite3.git
cd awtk-sqlite3; scons
```
## 嵌入式系统编译

将 src/sqlite3.c 加入工程。
