我来帮你分析该项目中 websocket 协议如何设置请求头。让我先搜索相关的 websocket 代码。

[14 tools called]

找到了！项目中有 `web_socket` 目录。让我查看这个目录下的文件：

[5 tools called]

现在让我搜索 websocket 的实现文件，看看具体如何设置请求头：

[8 tools called]

我看到项目使用了多个 WebSocket 实现库。让我查看更多相关文件：

[7 tools called]

很好！现在让我查找项目自己的 websocket 实现，看看它如何设置请求头。让我搜索 websocket 的实际实现代码：

[4 tools called]

太好了！我找到了 websocket 的使用示例。让我查看这些文件：

[2 tools called]

完美！我找到了关键信息。现在让我搜索一下底层库的实现，看看这些参数如何组装成 HTTP 请求头：

[3 tools called]

根据代码分析，我现在为你详细说明该项目中 websocket 协议如何设置请求头：

## 📋 WebSocket 请求头设置方式总结

该项目提供了**两种** WebSocket 实现，各有不同的请求头设置方法：

---

### 1️⃣ **项目自定义的 WebSocket 实现**

#### 核心结构体定义

```92:130:include_lib/net/web_socket/websocket_api.h
typedef struct websocket_struct {
    void *sk_fd;
    void *lst_fd;
    int ping_thread_id;
    int recv_thread_id;
    char websocket_mode;
    char websocket_recvsub;
    u8 websocket_data_type;
    u8 send_data_use_seq;
    u16 port;
    struct sockaddr_in servaddr;
    struct sockaddr_in clientaddr;
    u8 *ip_or_url;
    const char *origin_str;
    const char *user_agent_str;
    u8 ip_addr[16];
    u8 key[32];
    u8 msg[MAX_MSG];
    u8 msg_write;
    u8 msg_read;
    u8 *recv_buf;
    u32 recv_buf_size;
    u64 recv_len;
    u32 recv_time_out;
    u32 payload_data_len;
    u32 payload_data_continue;
    struct websocket_req_head req_head;
    struct websockets_mbedtls websockets_mbtls_info;
    u16 websocket_valid;
    int (*_init)(struct websocket_struct *websocket_info);
    void (*_exit)(struct websocket_struct *websocket_info);
    int (*_handshack)(struct websocket_struct *websocket_info);
    void (*_heart_thread)(void *param);
    void (*_recv_thread)(void *param);
    int (*_recv)(struct websocket_struct *websocket_info);
    int (*_send)(struct websocket_struct *websocket_info, u8 *buf, int len, char type);
    void (*_recv_cb)(u8 *buf, u32 len, u8 type);
    int (*_exit_notify)(struct websocket_struct *websocket_info);
} WEBSOCKET_INFO;
```

#### 请求头相关字段

```85:90:include_lib/net/web_socket/websocket_api.h
struct websocket_req_head {
    u8 medthod[4];
    u8 file[1024];
    u8 host[32];
    u8 version[8];
};
```

#### 实际使用示例

```252:263:apps/common/network_protocols/websocket/xz_main.c
static int websockets_client_init(struct websocket_struct *websockets_info, u8 *url, const char *origin_str)
{
    websockets_info->ip_or_url = url;
    websockets_info->origin_str = origin_str;
    websockets_info->recv_time_out = 1000;

    int err = websockets_struct_check(sizeof(struct websocket_struct));
    if (err == FALSE) {
        return err;
    }
    return websockets_info->_init(websockets_info);
}
```

**设置方式：**

- **`origin_str`**: 设置 `Origin` 请求头，示例值：`"http://coolaf.com"`
- **`user_agent_str`**: 设置 `User-Agent` 请求头（虽然在示例代码中未使用，但结构体中已定义）

---

### 2️⃣ **nopoll 库实现（开源 WebSocket 库）**

#### 设置额外请求头的 API

```87:87:include_lib/net/nopoll/nopoll_conn_opts.h
void nopoll_conn_opts_set_extra_headers(noPollConnOpts *opts, const char *extra_headers);
```

#### 其他相关 API

```71:75:include_lib/net/nopoll/nopoll_conn_opts.h
void        nopoll_conn_opts_set_cookie(noPollConnOpts *opts, const char *cookie_content);

void        nopoll_conn_opts_skip_origin_check(noPollConnOpts *opts, nopoll_bool skip_check);

void        nopoll_conn_opts_add_origin_header(noPollConnOpts *opts, nopoll_bool add);
```

#### 使用示例

```49:78:lib/net/nopoll/example/nopoll_client.c
    opts = nopoll_conn_opts_new();
    if (opts == NULL) {
        printf("ERROR: nopoll_conn_opts_new failed\n");
        ret = -1;
        goto exit;
    }

    hp = gethostbyname(hostName);
    if (hp != NULL) {
        hostIp = inet_ntoa(*(struct in_addr *)hp->h_addr);
        printf("host ip:%s\n", hostIp);
    } else {
        printf("ERROR: get host ip err\n");
        goto exit;
    }

#if defined(WEBSOCKET_SSL_ENABLE)
    /* if use ssl, then set the certs */
    if (!nopoll_conn_opts_set_ssl_certs(opts, NULL, 0, NULL, 0, NULL, 0, \
                                        websocket_test_ca, strlen(websocket_test_ca) + 1)) {
        printf("nopoll_conn_opts_set_ssl_certs failed\n");
        goto exit;
    }

    /* set ssl verfy */
    nopoll_conn_opts_ssl_peer_verify(opts, nopoll_true);
    conn = nopoll_conn_tls_new(ctx, opts, hostIp, hostPort, hostName, pathUrl, NULL, NULL);
#else
    conn = nopoll_conn_new_opts(ctx, opts, hostIp, hostPort, hostName, pathUrl, NULL, NULL);
#endif // WEBSOCKET_SSL_ENABLE
```

**nopoll 库的设置方式：**

- **`nopoll_conn_opts_set_extra_headers()`**: 设置自定义请求头（多个请求头用 `\r\n` 分隔）
- **`nopoll_conn_opts_set_cookie()`**: 设置 Cookie 请求头
- **`nopoll_conn_opts_add_origin_header()`**: 控制是否自动添加 Origin 头
- **`nopoll_conn_opts_skip_origin_check()`**: 跳过 Origin 校验

---

## 🔧 WebSocket 握手流程

根据代码分析，握手流程如下：

1. **初始化** WebSocket 结构体
2. **设置请求头参数**（`origin_str`、`user_agent_str` 等）
3. **调用握手函数** `_handshack()`
4. 内部会构造标准的 WebSocket 握手请求：

```http
GET /path HTTP/1.1
Host: example.com
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Key: [base64编码的随机key]
Sec-WebSocket-Version: 13
Origin: http://coolaf.com
User-Agent: [可选]
[其他自定义头]
```

---

## 📌 关键要点

1. **两种实现方式**：

   - 自定义实现：通过结构体字段直接设置
   - nopoll 库：通过 API 函数设置

2. **请求头组装**：

   - 底层实现会自动添加标准 WebSocket 握手头（Upgrade、Connection、Sec-WebSocket-Key 等）
   - 应用层只需设置 Origin 和其他自定义头

3. **安全特性**：

   - 使用 SHA-1 和 Base64 处理密钥
   - 支持 SSL/TLS 加密（wss://）

希望这个分析对你有帮助！如需查看更详细的底层实现，可以反编译库文件。
