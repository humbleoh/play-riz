# SSL/TLS 支持文档

本项目已添加对 SSL/TLS 加密连接的支持，可以确保 MQTT 通信的安全性。

## 功能特性

- ✅ 支持 TLS/SSL 加密连接
- ✅ 支持客户端证书认证（双向认证）
- ✅ 支持自定义 CA 证书
- ✅ 支持 TLS 版本配置
- ✅ 支持加密套件配置
- ✅ 支持主机名验证
- ✅ 兼容现有的非加密连接

## 快速开始

### 1. 生成测试证书

```bash
# 运行证书生成脚本
chmod +x scripts/generate_ssl_certs.sh
./scripts/generate_ssl_certs.sh
```

这将在 `./certs/` 目录下生成以下文件：
- `ca.crt` - CA 根证书
- `ca.key` - CA 私钥
- `server.crt` - 服务器证书
- `server.key` - 服务器私钥
- `client.crt` - 客户端证书
- `client.key` - 客户端私钥

### 2. 配置 MQTT Broker

#### Mosquitto 配置示例

创建 `mosquitto_ssl.conf`：

```conf
# 基本配置
port 1883
listener 8883

# SSL/TLS 配置
cafile /path/to/certs/ca.crt
certfile /path/to/certs/server.crt
keyfile /path/to/certs/server.key

# 要求客户端证书（可选，用于双向认证）
require_certificate true
use_identity_as_username true

# TLS 版本
tls_version tlsv1.2

# 日志配置
log_dest stdout
log_type all
```

启动 Mosquitto：
```bash
mosquitto -c mosquitto_ssl.conf
```

### 3. 使用 SSL 配置

#### 方式一：使用配置文件

复制并修改 `config_ssl.example.json`：

```json
{
  "mqtt": {
    "host": "localhost",
    "port": 8883,
    "ssl": {
      "enabled": true,
      "ca_file": "./certs/ca.crt",
      "cert_file": "./certs/client.crt",
      "key_file": "./certs/client.key",
      "verify_peer": true,
      "verify_hostname": true,
      "tls_version": "tlsv1.2"
    }
  }
}
```

#### 方式二：代码中直接配置

```cpp
#include "mqtt_client.h"

// 创建 SSL 配置
SslConfig ssl_config;
ssl_config.enabled = true;
ssl_config.ca_file = "./certs/ca.crt";
ssl_config.cert_file = "./certs/client.crt";
ssl_config.key_file = "./certs/client.key";
ssl_config.verify_peer = true;
ssl_config.verify_hostname = true;
ssl_config.tls_version = "tlsv1.2";

// 创建支持 SSL 的 MQTT 客户端
MqttClient client("client_id", "localhost", 8883, 60, ssl_config);

// 连接
if (client.connect()) {
    std::cout << "SSL connection established!" << std::endl;
}
```

#### 服务端使用示例

```cpp
#include "server.h"

// 创建 SSL 配置
SslConfig ssl_config;
ssl_config.enabled = true;
ssl_config.ca_file = "./certs/ca.crt";
ssl_config.verify_peer = true;
ssl_config.verify_hostname = false;  // 服务端通常不验证主机名

// 创建支持 SSL 的服务端
Server server("monitoring_server", "localhost", 8883, ssl_config);
server.start();
```

#### 设备端使用示例

```cpp
#include "device.h"

// 创建 SSL 配置
SslConfig ssl_config;
ssl_config.enabled = true;
ssl_config.ca_file = "./certs/ca.crt";
ssl_config.cert_file = "./certs/client.crt";
ssl_config.key_file = "./certs/client.key";

// 创建支持 SSL 的设备
Device device("device_001", "sensor", "localhost", 8883, ssl_config);
device.start();
```

## SSL 配置参数说明

| 参数 | 类型 | 必需 | 说明 |
|------|------|------|------|
| `enabled` | bool | 是 | 是否启用 SSL/TLS |
| `ca_file` | string | 是 | CA 证书文件路径 |
| `cert_file` | string | 否 | 客户端证书文件路径（双向认证时需要） |
| `key_file` | string | 否 | 客户端私钥文件路径（双向认证时需要） |
| `key_password` | string | 否 | 私钥密码 |
| `verify_peer` | bool | 否 | 是否验证对端证书（默认 true） |
| `verify_hostname` | bool | 否 | 是否验证主机名（默认 true） |
| `ciphers` | string | 否 | 允许的加密套件 |
| `tls_version` | string | 否 | TLS 版本（tlsv1.2, tlsv1.3 等） |

## 安全建议

### 生产环境

1. **使用受信任的 CA 证书**
   - 不要使用自签名证书
   - 使用 Let's Encrypt 或商业 CA

2. **启用证书验证**
   ```cpp
   ssl_config.verify_peer = true;
   ssl_config.verify_hostname = true;
   ```

3. **使用强加密套件**
   ```cpp
   ssl_config.ciphers = "ECDHE+AESGCM:ECDHE+CHACHA20:DHE+AESGCM:DHE+CHACHA20:!aNULL:!MD5:!DSS";
   ```

4. **使用最新的 TLS 版本**
   ```cpp
   ssl_config.tls_version = "tlsv1.3";  // 推荐使用 TLS 1.3
   ```

5. **定期更新证书**
   - 设置证书过期提醒
   - 自动化证书更新流程

### 开发/测试环境

1. **可以使用自签名证书**
   ```cpp
   ssl_config.verify_peer = false;    // 可以关闭验证
   ssl_config.verify_hostname = false;
   ```

2. **使用生成脚本创建测试证书**
   ```bash
   ./scripts/generate_ssl_certs.sh
   ```

## 故障排除

### 常见错误

1. **证书验证失败**
   ```
   Error: certificate verify failed
   ```
   - 检查 CA 证书路径是否正确
   - 确认证书未过期
   - 检查证书链是否完整

2. **主机名验证失败**
   ```
   Error: hostname verification failed
   ```
   - 确认证书中的 CN 或 SAN 与连接的主机名匹配
   - 或者设置 `verify_hostname = false`

3. **私钥密码错误**
   ```
   Error: bad decrypt
   ```
   - 检查 `key_password` 是否正确
   - 确认私钥文件格式正确

4. **TLS 版本不支持**
   ```
   Error: unsupported protocol
   ```
   - 检查客户端和服务端支持的 TLS 版本
   - 尝试使用 `tlsv1.2` 而不是 `tlsv1.3`

### 调试方法

1. **启用详细日志**
   ```cpp
   // 在连接前启用 Mosquitto 日志
   mosquitto_log_callback_set(client, log_callback);
   ```

2. **使用 OpenSSL 工具测试**
   ```bash
   # 测试服务器证书
   openssl s_client -connect localhost:8883 -CAfile ./certs/ca.crt
   
   # 测试客户端证书
   openssl s_client -connect localhost:8883 -CAfile ./certs/ca.crt -cert ./certs/client.crt -key ./certs/client.key
   ```

3. **检查证书信息**
   ```bash
   # 查看证书详情
   openssl x509 -in ./certs/ca.crt -text -noout
   
   # 验证证书链
   openssl verify -CAfile ./certs/ca.crt ./certs/client.crt
   ```

## 示例程序

运行完整的 SSL 示例：

```bash
# 编译示例程序
g++ -o ssl_example examples/ssl_example.cpp -lmosquitto -ljsoncpp -pthread

# 运行服务端示例
./ssl_example server

# 运行设备端示例
./ssl_example device
```

## 性能考虑

1. **SSL 握手开销**
   - 初始连接时间会增加
   - 考虑使用连接池或长连接

2. **加密/解密开销**
   - CPU 使用率会略有增加
   - 现代硬件影响很小

3. **证书验证开销**
   - 首次连接时验证证书链
   - 后续连接可以复用会话

## 兼容性

- **Mosquitto**: >= 1.4.0
- **OpenSSL**: >= 1.0.2
- **TLS 版本**: 1.2, 1.3
- **操作系统**: Linux, macOS, Windows

## 更多资源

- [Mosquitto SSL/TLS 配置](https://mosquitto.org/man/mosquitto-tls-7.html)
- [OpenSSL 文档](https://www.openssl.org/docs/)
- [MQTT 安全最佳实践](https://www.hivemq.com/blog/mqtt-security-fundamentals-tls-ssl/)