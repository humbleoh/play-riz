#ifndef SERVER_H
#define SERVER_H

#include "mqtt_client.h"
#include <map>
#include <vector>
#include <chrono>
#include <memory>
#include <json/json.h>

/**
 * 设备状态信息结构
 */
struct DeviceStatus {
    std::string device_id;              // 设备ID
    std::string status;                 // 设备状态（online/offline/error）
    std::chrono::system_clock::time_point last_seen; // 最后活跃时间
    Json::Value properties;             // 设备属性
    
    DeviceStatus() : status("offline"), last_seen(std::chrono::system_clock::now()) {}
};

/**
 * 控制命令结构
 */
struct ControlCommand {
    std::string command_id;             // 命令ID
    std::string device_id;              // 目标设备ID
    std::string command_type;           // 命令类型
    Json::Value parameters;             // 命令参数
    std::chrono::system_clock::time_point timestamp; // 时间戳
    
    ControlCommand() : timestamp(std::chrono::system_clock::now()) {}
};

/**
 * 服务端框架类
 * 负责监测设备状态，发送控制命令，处理设备响应
 */
class Server {
public:
    // 设备状态变化回调函数类型
    using DeviceStatusCallback = std::function<void(const std::string& device_id, const DeviceStatus& status)>;
    // 命令响应回调函数类型
    using CommandResponseCallback = std::function<void(const std::string& command_id, const Json::Value& response)>;
    
    /**
     * 构造函数
     * @param server_id 服务端ID
     * @param mqtt_host MQTT服务器地址
     * @param mqtt_port MQTT服务器端口
     */
    Server(const std::string& server_id, 
           const std::string& mqtt_host = "localhost", 
           int mqtt_port = 1883);
    
    /**
     * 构造函数（支持SSL/TLS）
     * @param server_id 服务端ID
     * @param mqtt_host MQTT服务器地址
     * @param mqtt_port MQTT服务器端口
     * @param ssl_config SSL/TLS配置
     */
    Server(const std::string& server_id,
           const std::string& mqtt_host,
           int mqtt_port,
           const SslConfig& ssl_config);
    
    /**
     * 构造函数（支持身份验证）
     * @param server_id 服务端ID
     * @param mqtt_host MQTT服务器地址
     * @param mqtt_port MQTT服务器端口
     * @param auth_config 身份验证配置
     */
    Server(const std::string& server_id,
           const std::string& mqtt_host,
           int mqtt_port,
           const AuthConfig& auth_config);
    
    /**
     * 构造函数（支持SSL/TLS + 身份验证）
     * @param server_id 服务端ID
     * @param mqtt_host MQTT服务器地址
     * @param mqtt_port MQTT服务器端口
     * @param ssl_config SSL/TLS配置
     * @param auth_config 身份验证配置
     */
    Server(const std::string& server_id,
           const std::string& mqtt_host,
           int mqtt_port,
           const SslConfig& ssl_config,
           const AuthConfig& auth_config);
    
    /**
     * 析构函数
     */
    ~Server();
    
    /**
     * 启动服务端
     * @return 启动是否成功
     */
    bool start();
    
    /**
     * 停止服务端
     */
    void stop();
    
    /**
     * 发送控制命令到指定设备
     * @param device_id 设备ID
     * @param command_type 命令类型
     * @param parameters 命令参数
     * @return 命令ID（用于跟踪响应）
     */
    std::string sendCommand(const std::string& device_id, 
                           const std::string& command_type, 
                           const Json::Value& parameters = Json::Value());
    
    /**
     * 获取设备状态
     * @param device_id 设备ID
     * @return 设备状态，如果设备不存在返回nullptr
     */
    std::shared_ptr<DeviceStatus> getDeviceStatus(const std::string& device_id) const;
    
    /**
     * 获取所有设备状态
     * @return 设备状态映射
     */
    std::map<std::string, DeviceStatus> getAllDeviceStatus() const;
    
    /**
     * 获取在线设备列表
     * @return 在线设备ID列表
     */
    std::vector<std::string> getOnlineDevices() const;
    
    /**
     * 设置设备状态变化回调
     * @param callback 回调函数
     */
    void setDeviceStatusCallback(DeviceStatusCallback callback);
    
    /**
     * 设置命令响应回调
     * @param callback 回调函数
     */
    void setCommandResponseCallback(CommandResponseCallback callback);
    
    /**
     * 设置设备离线超时时间
     * @param timeout_seconds 超时时间（秒）
     */
    void setDeviceTimeout(int timeout_seconds);
    
    /**
     * 请求设备状态更新
     * @param device_id 设备ID，为空则请求所有设备
     */
    void requestDeviceStatus(const std::string& device_id = "");

private:
    /**
     * 处理MQTT消息
     * @param topic 主题
     * @param payload 消息内容
     */
    void handleMessage(const std::string& topic, const std::string& payload);
    
    /**
     * 处理设备状态消息
     * @param device_id 设备ID
     * @param payload 消息内容
     */
    void handleDeviceStatus(const std::string& device_id, const std::string& payload);
    
    /**
     * 处理命令响应消息
     * @param device_id 设备ID
     * @param payload 消息内容
     */
    void handleCommandResponse(const std::string& device_id, const std::string& payload);
    
    /**
     * 处理设备心跳消息
     * @param device_id 设备ID
     * @param payload 消息内容
     */
    void handleDeviceHeartbeat(const std::string& device_id, const std::string& payload);
    
    /**
     * 设备超时检查线程函数
     */
    void deviceTimeoutCheck();
    
    /**
     * 生成唯一命令ID
     * @return 命令ID
     */
    std::string generateCommandId();
    
    /**
     * 解析主题获取设备ID
     * @param topic 主题
     * @return 设备ID，解析失败返回空字符串
     */
    std::string parseDeviceIdFromTopic(const std::string& topic);
    
private:
    std::string m_server_id;                        // 服务端ID
    std::unique_ptr<MqttClient> m_mqtt_client;      // MQTT客户端
    
    std::map<std::string, DeviceStatus> m_devices;  // 设备状态映射
    std::map<std::string, ControlCommand> m_pending_commands; // 待响应命令
    
    DeviceStatusCallback m_device_status_callback;  // 设备状态回调
    CommandResponseCallback m_command_response_callback; // 命令响应回调
    
    std::atomic<bool> m_running;                    // 运行状态
    int m_device_timeout;                           // 设备超时时间（秒）
    
    std::thread m_timeout_check_thread;             // 超时检查线程
    mutable std::mutex m_devices_mutex;             // 设备状态互斥锁
    mutable std::mutex m_commands_mutex;            // 命令互斥锁
    
    std::atomic<uint64_t> m_command_counter;        // 命令计数器
    
    // MQTT主题定义
    static const std::string TOPIC_DEVICE_STATUS;   // 设备状态主题
    static const std::string TOPIC_DEVICE_COMMAND;  // 设备命令主题
    static const std::string TOPIC_DEVICE_RESPONSE; // 设备响应主题
    static const std::string TOPIC_DEVICE_HEARTBEAT; // 设备心跳主题
};

#endif // SERVER_H