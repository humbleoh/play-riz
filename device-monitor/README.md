# MQTT设备监控框架

这是一个基于C++和MQTT协议的简单设备监控框架，包含服务端和设备端两个组件。

## 功能特性

### 服务端功能
- 建立并维护MQTT连接
- 实时监测设备状态变化
- 发送控制命令到设备
- 处理设备响应消息
- 设备超时检测和管理
- 交互式命令行界面

### 设备端功能
- 建立并维护MQTT连接
- 接收并执行控制命令
- 定期上报设备状态
- 发送心跳消息
- 自动重连机制
- 模拟设备数据生成

## 依赖库

- **libmosquitto**: MQTT客户端库
- **jsoncpp**: JSON解析库
- **pthread**: 线程库

> **注意**: 在IDE中可能会显示 `mosquitto.h` 和 `json/json.h` 找不到的错误，这是正常现象。这些是外部依赖库的头文件，需要在系统中安装相应的开发库后，通过CMake构建系统才能正确找到。项目在正确安装依赖并编译后可以正常运行。

### 安装依赖 (macOS)

```bash
# 使用Homebrew安装
brew install mosquitto jsoncpp
```

### 安装依赖 (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install libmosquitto-dev libjsoncpp-dev
```

## 编译和构建

```bash
# 创建构建目录
mkdir build
cd build

# 配置CMake
cmake ..

# 编译
make

# 安装（可选）
sudo make install
```

## MQTT主题结构

### 设备状态相关
- `device/{device_id}/status` - 设备状态上报
- `device/{device_id}/heartbeat` - 设备心跳
- `server/status_request/{device_id}` - 服务端请求设备状态

### 命令控制相关
- `device/{device_id}/command` - 服务端发送命令
- `device/{device_id}/response` - 设备响应命令结果

## 消息格式

### 设备状态消息
```json
{
  "device_id": "device001",
  "timestamp": 1640995200,
  "status": "online",
  "properties": {
    "temperature": 25.5,
    "humidity": 60.2,
    "battery": 85
  }
}
```

### 控制命令消息
```json
{
  "command_id": "cmd_001",
  "command": "set_temperature",
  "parameters": {
    "target_temp": 22.0
  },
  "timestamp": 1640995200
}
```

### 命令响应消息
```json
{
  "command_id": "cmd_001",
  "success": true,
  "message": "Temperature set successfully",
  "timestamp": 1640995210
}
```

## 使用方法

### 启动MQTT Broker

```bash
# 启动本地Mosquitto broker
mosquitto -v
```

### 运行服务端

```bash
# 基本用法
./server

# 指定MQTT服务器和参数
./server --host localhost --port 1883 --server-id server001 --timeout 30
```

#### 服务端命令行选项
- `--host`: MQTT服务器地址 (默认: localhost)
- `--port`: MQTT服务器端口 (默认: 1883)
- `--server-id`: 服务器ID (默认: server001)
- `--timeout`: 设备超时时间，秒 (默认: 30)

#### 服务端交互命令
- `list` - 列出所有已知设备
- `status <device_id>` - 查看指定设备状态
- `command <device_id> <command> [params]` - 发送命令到设备
- `refresh <device_id>` - 刷新设备状态
- `quit` - 退出程序

### 运行设备端

```bash
# 基本用法
./device --id device001

# 完整参数示例
./device --id device001 --type sensor --host localhost --port 1883 --status-interval 10 --heartbeat-interval 5 --simulate
```

#### 设备端命令行选项
- `--id`: 设备ID (必需)
- `--type`: 设备类型 (默认: generic)
- `--host`: MQTT服务器地址 (默认: localhost)
- `--port`: MQTT服务器端口 (默认: 1883)
- `--status-interval`: 状态上报间隔，秒 (默认: 10)
- `--heartbeat-interval`: 心跳间隔，秒 (默认: 5)
- `--simulate`: 启用模拟数据模式

#### 设备端交互命令
- `status` - 显示当前设备状态
- `set <property> <value>` - 设置设备属性
- `get <property>` - 获取设备属性值
- `list` - 列出所有设备属性
- `quit` - 退出程序

## 使用示例

### 示例1：基本监控

1. 启动MQTT broker:
```bash
mosquitto -v
```

2. 启动服务端:
```bash
./server
```

3. 启动设备端:
```bash
./device --id sensor001 --type temperature_sensor --simulate
```

4. 在服务端控制台中:
```
> list
> status sensor001
> command sensor001 get_temperature
```

### 示例2：多设备监控

启动多个设备实例:
```bash
# 终端1
./device --id sensor001 --type temperature --simulate

# 终端2  
./device --id sensor002 --type humidity --simulate

# 终端3
./device --id actuator001 --type motor
```

在服务端可以同时监控所有设备。

## 错误处理

框架包含以下错误处理机制：

1. **连接错误**: 自动重连MQTT服务器
2. **消息解析错误**: 记录错误日志并继续运行
3. **设备超时**: 自动标记设备为离线状态
4. **命令执行错误**: 返回错误响应给服务端

## 扩展开发

### 添加新的设备命令

在设备端代码中注册新的命令处理器:

```cpp
device.registerCommandHandler("custom_command", [](const Json::Value& params) -> CommandResult {
    CommandResult result;
    // 实现自定义命令逻辑
    result.success = true;
    result.message = "Command executed successfully";
    return result;
});
```

### 添加新的设备属性

```cpp
device.setProperty("custom_property", 42.0);
```

## 注意事项

1. 确保MQTT broker正在运行
2. 检查防火墙设置，确保MQTT端口(1883)可访问
3. 设备ID必须唯一
4. 建议在生产环境中使用TLS加密连接
5. 根据实际需求调整心跳和状态上报间隔

## 许可证

本项目采用MIT许可证。