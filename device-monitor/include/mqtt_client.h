#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <mosquitto.h>
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>

/**
 * SSL/TLS配置结构体
 */
struct SslConfig {
    bool enabled = false;                    // 是否启用SSL/TLS
    std::string ca_file;                     // CA证书文件路径
    std::string cert_file;                   // 客户端证书文件路径
    std::string key_file;                    // 客户端私钥文件路径
    std::string key_password;                // 私钥密码
    bool verify_peer = true;                 // 是否验证服务器证书
    bool verify_hostname = true;             // 是否验证主机名
    std::string ciphers;                     // 允许的加密套件
    std::string tls_version = "tlsv1.2";      // TLS版本
};

/**
 * 身份验证配置结构体
 */
struct AuthConfig {
    bool enabled = false;                    // 是否启用身份验证
    std::string username;                    // 用户名
    std::string password;                    // 密码
};

/**
 * MQTT客户端基础类
 * 提供MQTT连接、消息发布/订阅、重连等基础功能
 */
class MqttClient {
public:
    // 消息回调函数类型定义
    using MessageCallback = std::function<void(const std::string& topic, const std::string& payload)>;
    using ConnectionCallback = std::function<void(bool connected)>;
    
    /**
     * 构造函数
     * @param client_id 客户端ID
     * @param host MQTT服务器地址
     * @param port MQTT服务器端口
     * @param keep_alive 保活时间（秒）
     */
    MqttClient(const std::string& client_id, 
               const std::string& host = "localhost", 
               int port = 1883, 
               int keep_alive = 60);
    
    /**
     * 构造函数（支持SSL/TLS）
     * @param client_id 客户端ID
     * @param host MQTT服务器地址
     * @param port MQTT服务器端口
     * @param ssl_config SSL/TLS配置
     * @param keep_alive 保活时间（秒）
     */
    MqttClient(const std::string& client_id,
               const std::string& host,
               int port,
               const SslConfig& ssl_config,
               int keep_alive = 60);
    
    /**
     * 构造函数（支持身份验证）
     * @param client_id 客户端ID
     * @param host MQTT服务器地址
     * @param port MQTT服务器端口
     * @param auth_config 身份验证配置
     * @param keep_alive 保活时间（秒）
     */
    MqttClient(const std::string& client_id,
               const std::string& host,
               int port,
               const AuthConfig& auth_config,
               int keep_alive = 60);
    
    /**
     * 构造函数（支持SSL/TLS和身份验证）
     * @param client_id 客户端ID
     * @param host MQTT服务器地址
     * @param port MQTT服务器端口
     * @param ssl_config SSL/TLS配置
     * @param auth_config 身份验证配置
     * @param keep_alive 保活时间（秒）
     */
    MqttClient(const std::string& client_id,
               const std::string& host,
               int port,
               const SslConfig& ssl_config,
               const AuthConfig& auth_config,
               int keep_alive = 60);
    
    /**
     * 析构函数
     */
    virtual ~MqttClient();
    
    /**
     * 连接到MQTT服务器
     * @return 连接是否成功
     */
    bool connect();
    
    /**
     * 断开MQTT连接
     */
    void disconnect();
    
    /**
     * 发布消息
     * @param topic 主题
     * @param payload 消息内容
     * @param qos 服务质量等级
     * @param retain 是否保留消息
     * @return 发布是否成功
     */
    bool publish(const std::string& topic, 
                const std::string& payload, 
                int qos = 0, 
                bool retain = false);
    
    /**
     * 订阅主题
     * @param topic 主题
     * @param qos 服务质量等级
     * @return 订阅是否成功
     */
    bool subscribe(const std::string& topic, int qos = 0);
    
    /**
     * 取消订阅主题
     * @param topic 主题
     * @return 取消订阅是否成功
     */
    bool unsubscribe(const std::string& topic);
    
    /**
     * 设置消息接收回调函数
     * @param callback 回调函数
     */
    void setMessageCallback(MessageCallback callback);
    
    /**
     * 设置连接状态回调函数
     * @param callback 回调函数
     */
    void setConnectionCallback(ConnectionCallback callback);
    
    /**
     * 启动客户端（开始消息循环）
     */
    void start();
    
    /**
     * 停止客户端
     */
    void stop();
    
    /**
     * 检查是否已连接
     * @return 连接状态
     */
    bool isConnected() const;
    
    /**
     * 设置自动重连
     * @param enable 是否启用自动重连
     * @param retry_interval 重连间隔（秒）
     */
    void setAutoReconnect(bool enable, int retry_interval = 5);
    
    /**
     * 配置SSL/TLS
     * @param ssl_config SSL/TLS配置
     * @return 配置是否成功
     */
    bool configureSsl(const SslConfig& ssl_config);
    
    /**
     * 获取当前SSL配置
     * @return SSL配置
     */
    const SslConfig& getSslConfig() const;
    
    /**
     * 配置身份验证
     * @param auth_config 身份验证配置
     * @return 配置是否成功
     */
    bool configureAuth(const AuthConfig& auth_config);
    
    /**
     * 获取当前身份验证配置
     * @return 身份验证配置
     */
    const AuthConfig& getAuthConfig() const;

protected:
    // MQTT回调函数
    static void onConnect(struct mosquitto* mosq, void* userdata, int result);
    static void onDisconnect(struct mosquitto* mosq, void* userdata, int result);
    static void onMessage(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message);
    static void onSubscribe(struct mosquitto* mosq, void* userdata, int mid, int qos_count, const int* granted_qos);
    static void onUnsubscribe(struct mosquitto* mosq, void* userdata, int mid);
    static void onPublish(struct mosquitto* mosq, void* userdata, int mid);
    
    // 重连线程函数
    void reconnectLoop();
    
private:
    struct mosquitto* m_mosquitto;          // mosquitto客户端实例
    std::string m_client_id;                // 客户端ID
    std::string m_host;                     // 服务器地址
    int m_port;                             // 服务器端口
    int m_keep_alive;                       // 保活时间
    SslConfig m_ssl_config;                 // SSL/TLS配置
    AuthConfig m_auth_config;               // 身份验证配置
    
    std::atomic<bool> m_connected;          // 连接状态
    std::atomic<bool> m_running;            // 运行状态
    std::atomic<bool> m_auto_reconnect;     // 自动重连开关
    int m_retry_interval;                   // 重连间隔
    
    MessageCallback m_message_callback;     // 消息回调
    ConnectionCallback m_connection_callback; // 连接状态回调
    
    std::thread m_loop_thread;              // 消息循环线程
    std::thread m_reconnect_thread;         // 重连线程
    
    mutable std::mutex m_mutex;             // 互斥锁
    std::condition_variable m_cv;           // 条件变量
    
    static bool s_lib_initialized;          // 库初始化标志
    static std::mutex s_init_mutex;         // 初始化互斥锁
};

#endif // MQTT_CLIENT_H