#ifndef DEVICE_H
#define DEVICE_H

#include "mqtt_client.h"
#include <map>
#include <atomic>
#include <thread>
#include <chrono>
#include <json/json.h>

/**
 * 设备属性结构
 */
struct DeviceProperty {
    std::string name;                   // 属性名称
    Json::Value value;                  // 属性值
    std::string unit;                   // 单位
    bool writable;                      // 是否可写
    
    DeviceProperty() : writable(false) {}
    DeviceProperty(const std::string& n, const Json::Value& v, const std::string& u = "", bool w = false)
        : name(n), value(v), unit(u), writable(w) {}
};

/**
 * 命令执行结果结构
 */
struct CommandResult {
    std::string command_id;             // 命令ID
    bool success;                       // 执行是否成功
    std::string error_message;          // 错误信息
    Json::Value result_data;            // 结果数据
    std::chrono::system_clock::time_point timestamp; // 时间戳
    
    CommandResult() : success(false), timestamp(std::chrono::system_clock::now()) {}
};

/**
 * 设备端框架类
 * 负责接收控制命令，上报状态信息，处理异常情况
 */
class Device {
public:
    // 命令处理回调函数类型
    using CommandHandler = std::function<CommandResult(const std::string& command_type, const Json::Value& parameters)>;
    // 状态更新回调函数类型
    using StatusUpdateCallback = std::function<void(const std::string& device_id)>;
    
    /**
     * 构造函数
     * @param device_id 设备ID
     * @param device_type 设备类型
     * @param mqtt_host MQTT服务器地址
     * @param mqtt_port MQTT服务器端口
     */
    Device(const std::string& device_id, 
           const std::string& device_type,
           const std::string& mqtt_host = "localhost", 
           int mqtt_port = 1883);
    
    /**
     * 构造函数（支持SSL/TLS）
     * @param device_id 设备ID
     * @param device_type 设备类型
     * @param mqtt_host MQTT服务器地址
     * @param mqtt_port MQTT服务器端口
     * @param ssl_config SSL/TLS配置
     */
    Device(const std::string& device_id,
           const std::string& device_type,
           const std::string& mqtt_host,
           int mqtt_port,
           const SslConfig& ssl_config);
    
    /**
     * 构造函数（支持用户名/密码认证）
     * @param device_id 设备ID
     * @param device_type 设备类型
     * @param mqtt_host MQTT服务器地址
     * @param mqtt_port MQTT服务器端口
     * @param auth_config 认证配置
     */
    Device(const std::string& device_id,
           const std::string& device_type,
           const std::string& mqtt_host,
           int mqtt_port,
           const AuthConfig& auth_config);
    
    /**
     * 构造函数（支持SSL/TLS + 用户名/密码认证）
     * @param device_id 设备ID
     * @param device_type 设备类型
     * @param mqtt_host MQTT服务器地址
     * @param mqtt_port MQTT服务器端口
     * @param ssl_config SSL/TLS配置
     * @param auth_config 认证配置
     */
    Device(const std::string& device_id,
           const std::string& device_type,
           const std::string& mqtt_host,
           int mqtt_port,
           const SslConfig& ssl_config,
           const AuthConfig& auth_config);
    
    /**
     * 析构函数
     */
    ~Device();
    
    /**
     * 启动设备
     * @return 启动是否成功
     */
    bool start();
    
    /**
     * 停止设备
     */
    void stop();
    
    /**
     * 设置设备属性
     * @param name 属性名称
     * @param value 属性值
     * @param unit 单位
     * @param writable 是否可写
     */
    void setProperty(const std::string& name, 
                    const Json::Value& value, 
                    const std::string& unit = "", 
                    bool writable = false);
    
    /**
     * 获取设备属性
     * @param name 属性名称
     * @return 属性值，如果属性不存在返回null
     */
    Json::Value getProperty(const std::string& name) const;
    
    /**
     * 更新设备属性值
     * @param name 属性名称
     * @param value 新值
     * @return 更新是否成功
     */
    bool updateProperty(const std::string& name, const Json::Value& value);
    
    /**
     * 获取所有属性
     * @return 属性映射
     */
    std::map<std::string, DeviceProperty> getAllProperties() const;
    
    /**
     * 注册命令处理器
     * @param command_type 命令类型
     * @param handler 处理函数
     */
    void registerCommandHandler(const std::string& command_type, CommandHandler handler);
    
    /**
     * 设置状态更新回调
     * @param callback 回调函数
     */
    void setStatusUpdateCallback(StatusUpdateCallback callback);
    
    /**
     * 手动上报设备状态
     */
    void reportStatus();
    
    /**
     * 设置状态上报间隔
     * @param interval_seconds 间隔时间（秒）
     */
    void setStatusReportInterval(int interval_seconds);
    
    /**
     * 设置心跳间隔
     * @param interval_seconds 间隔时间（秒）
     */
    void setHeartbeatInterval(int interval_seconds);
    
    /**
     * 获取设备ID
     * @return 设备ID
     */
    const std::string& getDeviceId() const { return m_device_id; }
    
    /**
     * 获取设备类型
     * @return 设备类型
     */
    const std::string& getDeviceType() const { return m_device_type; }
    
    /**
     * 设置设备状态
     * @param status 状态（online/offline/error/maintenance）
     */
    void setDeviceStatus(const std::string& status);
    
    /**
     * 获取设备状态
     * @return 当前状态
     */
    const std::string& getDeviceStatus() const { return m_device_status; }

private:
    /**
     * 处理MQTT消息
     * @param topic 主题
     * @param payload 消息内容
     */
    void handleMessage(const std::string& topic, const std::string& payload);
    
    /**
     * 处理控制命令
     * @param payload 命令内容
     */
    void handleCommand(const std::string& payload);
    
    /**
     * 处理状态请求
     * @param payload 请求内容
     */
    void handleStatusRequest(const std::string& payload);
    
    /**
     * 发送命令响应
     * @param result 命令执行结果
     */
    void sendCommandResponse(const CommandResult& result);
    
    /**
     * 状态上报线程函数
     */
    void statusReportLoop();
    
    /**
     * 心跳线程函数
     */
    void heartbeatLoop();
    
    /**
     * 构建状态消息
     * @return JSON格式的状态消息
     */
    Json::Value buildStatusMessage();
    
    /**
     * 构建心跳消息
     * @return JSON格式的心跳消息
     */
    Json::Value buildHeartbeatMessage();
    
    /**
     * 处理连接状态变化
     * @param connected 是否已连接
     */
    void handleConnectionChange(bool connected);

private:
    std::string m_device_id;                        // 设备ID
    std::string m_device_type;                      // 设备类型
    std::string m_device_status;                    // 设备状态
    std::unique_ptr<MqttClient> m_mqtt_client;      // MQTT客户端
    
    std::map<std::string, DeviceProperty> m_properties; // 设备属性
    std::map<std::string, CommandHandler> m_command_handlers; // 命令处理器
    
    StatusUpdateCallback m_status_update_callback;  // 状态更新回调
    
    std::atomic<bool> m_running;                    // 运行状态
    int m_status_report_interval;                   // 状态上报间隔（秒）
    int m_heartbeat_interval;                       // 心跳间隔（秒）
    
    std::thread m_status_report_thread;             // 状态上报线程
    std::thread m_heartbeat_thread;                 // 心跳线程
    
    mutable std::mutex m_properties_mutex;          // 属性互斥锁
    mutable std::mutex m_handlers_mutex;            // 处理器互斥锁
    
    std::chrono::system_clock::time_point m_start_time; // 启动时间
    
    // MQTT主题定义
    std::string m_topic_command;                    // 命令接收主题
    std::string m_topic_status;                     // 状态上报主题
    std::string m_topic_response;                   // 响应发送主题
    std::string m_topic_heartbeat;                  // 心跳发送主题
    std::string m_topic_status_request;             // 状态请求主题
};

#endif // DEVICE_H