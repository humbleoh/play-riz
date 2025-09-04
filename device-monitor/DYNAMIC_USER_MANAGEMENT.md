# 动态用户管理文档

本文档介绍如何在MQTT设备监控系统中动态添加和管理用户认证，实现用户的即时生效。

## 功能概述

✅ **动态添加用户** - 无需重启broker即可添加新用户  
✅ **动态删除用户** - 无需重启broker即可删除用户  
✅ **即时生效** - 用户添加/删除后立即可用/禁用  
✅ **用户列表管理** - 查看当前所有用户  
✅ **状态监控** - 检查broker和认证系统状态  

## 技术原理

### Mosquitto动态重载机制

Mosquitto broker支持通过SIGHUP信号动态重载配置文件，包括：<mcreference link="https://mosquitto.org/documentation/authentication-methods/" index="1">1</mcreference>
- 密码文件 (`password_file`)
- ACL访问控制列表 (`acl_file`) 
- SSL/TLS证书文件

当收到SIGHUP信号时，broker会：<mcreference link="https://mosquitto.org/man/mosquitto-8.html" index="5">5</mcreference>
1. 重新读取配置文件
2. 重载密码文件中的用户凭据
3. 重载ACL权限设置
4. 重载SSL证书（如果使用）
5. 保持现有连接不中断

### 实现方式

```bash
# 1. 使用mosquitto_passwd工具修改密码文件
mosquitto_passwd mosquitto_passwd newuser

# 2. 发送SIGHUP信号重载配置
kill -HUP <mosquitto_pid>
```

## 使用方法

### 用户管理工具

项目提供了 `manage_users.sh` 脚本来简化用户管理操作：

```bash
# 查看帮助
./manage_users.sh -h

# 添加用户（交互式输入密码）
./manage_users.sh -a username

# 添加用户（命令行指定密码）
./manage_users.sh -a username password

# 删除用户
./manage_users.sh -d username

# 列出所有用户
./manage_users.sh -l

# 检查broker状态
./manage_users.sh -s

# 手动重载配置
./manage_users.sh -r
```

### 实际操作示例

#### 1. 添加新用户

```bash
$ ./manage_users.sh -a newuser newpass123
添加用户: newuser
✓ 用户添加成功
正在重载broker配置 (PID: 37516)...
✓ 配置重载成功
✓ 用户 'newuser' 已生效，可以立即使用
```

#### 2. 验证用户可用性

```bash
# 立即测试新用户连接
$ mosquitto_pub -h localhost -p 1883 -u newuser -P newpass123 -t test/topic -m "Hello"
# 成功发布消息，说明用户已生效
```

#### 3. 查看用户列表

```bash
$ ./manage_users.sh -l
当前用户列表:
==================
1. testuser
2. device01
3. newuser
==================
总计: 3 个用户
```

#### 4. 删除用户

```bash
$ ./manage_users.sh -d newuser
删除用户: newuser
✓ 用户删除成功
正在重载broker配置 (PID: 37516)...
✓ 配置重载成功
✓ 用户 'newuser' 已被禁用，立即生效
```

## 高级功能

### 1. 批量用户管理

可以创建批量操作脚本：

```bash
#!/bin/bash
# 批量添加用户
users=("user1:pass1" "user2:pass2" "user3:pass3")

for user_info in "${users[@]}"; do
    IFS=':' read -r username password <<< "$user_info"
    ./manage_users.sh -a "$username" "$password"
done
```

### 2. 用户权限管理

结合ACL文件可以实现细粒度权限控制：

```conf
# mosquitto_auth.conf 中添加
acl_file /path/to/acl_file

# acl_file 内容示例
# 默认拒绝所有访问
topic deny #

# 为特定用户授权
user testuser
topic readwrite device/+/status
topic readwrite device/+/command

user device01
topic readwrite device/device01/#
```

### 3. 自动化集成

可以将用户管理集成到现有系统中：

```python
# Python示例：通过API动态添加用户
import subprocess

def add_mqtt_user(username, password):
    """动态添加MQTT用户"""
    try:
        result = subprocess.run([
            './manage_users.sh', '-a', username, password
        ], capture_output=True, text=True, check=True)
        return True, result.stdout
    except subprocess.CalledProcessError as e:
        return False, e.stderr

# 使用示例
success, message = add_mqtt_user('api_user', 'secure_password')
if success:
    print(f"用户添加成功: {message}")
else:
    print(f"用户添加失败: {message}")
```

## 安全考虑

### 1. 密码安全

- 使用强密码策略
- 定期更换密码
- 避免在命令行中明文传递密码

```bash
# 推荐：交互式输入密码
./manage_users.sh -a username
# 系统会提示输入密码，不会在命令历史中留下记录

# 不推荐：命令行明文密码
./manage_users.sh -a username plaintext_password
```

### 2. 权限控制

- 限制用户管理脚本的执行权限
- 使用ACL控制用户的topic访问权限
- 定期审计用户列表

### 3. 监控和日志

```bash
# 启用详细日志记录
log_type all
log_dest file /var/log/mosquitto/mosquitto.log

# 监控认证事件
log_type subscribe
log_type unsubscribe
log_type websockets
log_type notice
log_type information
log_type warning
log_type error
log_type debug
```

## 故障排除

### 常见问题

1. **配置重载失败**
   ```bash
   # 检查broker进程状态
   ./manage_users.sh -s
   
   # 手动重载
   ./manage_users.sh -r
   ```

2. **用户添加后无法连接**
   ```bash
   # 检查密码文件格式
   cat mosquitto_passwd
   
   # 验证broker配置
   mosquitto -c mosquitto_auth.conf -v
   ```

3. **权限不足**
   ```bash
   # 确保脚本有执行权限
   chmod +x manage_users.sh
   
   # 确保密码文件可写
   ls -la mosquitto_passwd
   ```

### 调试技巧

```bash
# 启用详细日志模式测试
mosquitto -c mosquitto_auth.conf -v

# 使用mosquitto_sub测试用户连接
mosquitto_sub -h localhost -p 1883 -u testuser -P testpass -t '#' -v

# 检查系统信号发送
ps aux | grep mosquitto
kill -HUP <pid>
```

## 性能考虑

- **重载频率**: 避免频繁重载配置，建议批量操作
- **用户数量**: 密码文件适合中小规模用户管理（<1000用户）
- **大规模部署**: 考虑使用数据库认证插件或动态安全插件<mcreference link="https://mosquitto.org/documentation/authentication-methods/" index="1">1</mcreference>

## 总结

通过Mosquitto的SIGHUP信号机制，可以实现：

✅ **零停机时间** - 无需重启broker  
✅ **即时生效** - 用户添加/删除立即可用  
✅ **连接保持** - 现有客户端连接不受影响  
✅ **操作简便** - 提供便捷的管理工具  
✅ **安全可靠** - 支持密码加密和权限控制  

这使得MQTT设备监控系统能够灵活应对动态的用户管理需求，特别适合IoT设备的动态接入场景。