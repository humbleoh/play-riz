#include "mqtt_client.h"
#include <iostream>
#include <sstream>
#include <cstring>

// 静态成员初始化
bool MqttClient::s_lib_initialized = false;
std::mutex MqttClient::s_init_mutex;

MqttClient::MqttClient(const std::string& client_id, 
                       const std::string& host, 
                       int port, 
                       int keep_alive)
    : m_client_id(client_id)
    , m_host(host)
    , m_port(port)
    , m_keep_alive(keep_alive)
    , m_connected(false)
    , m_running(false)
    , m_auto_reconnect(false)
    , m_retry_interval(5)
    , m_mosquitto(nullptr)
{
    // 初始化mosquitto库（线程安全）
    std::lock_guard<std::mutex> lock(s_init_mutex);
    if (!s_lib_initialized) {
        mosquitto_lib_init();
        s_lib_initialized = true;
    }
    
    // 创建mosquitto客户端实例
    m_mosquitto = mosquitto_new(m_client_id.c_str(), true, this);
    if (!m_mosquitto) {
        throw std::runtime_error("Failed to create mosquitto client");
    }
    
    // 设置回调函数
    mosquitto_connect_callback_set(m_mosquitto, onConnect);
    mosquitto_disconnect_callback_set(m_mosquitto, onDisconnect);
    mosquitto_message_callback_set(m_mosquitto, onMessage);
    mosquitto_subscribe_callback_set(m_mosquitto, onSubscribe);
    mosquitto_unsubscribe_callback_set(m_mosquitto, onUnsubscribe);
    mosquitto_publish_callback_set(m_mosquitto, onPublish);
}

MqttClient::MqttClient(const std::string& client_id,
                       const std::string& host,
                       int port,
                       const SslConfig& ssl_config,
                       int keep_alive)
    : m_client_id(client_id)
    , m_host(host)
    , m_port(port)
    , m_keep_alive(keep_alive)
    , m_ssl_config(ssl_config)
    , m_connected(false)
    , m_running(false)
    , m_auto_reconnect(false)
    , m_retry_interval(5)
    , m_mosquitto(nullptr)
{
    // 初始化mosquitto库（线程安全）
    std::lock_guard<std::mutex> lock(s_init_mutex);
    if (!s_lib_initialized) {
        mosquitto_lib_init();
        s_lib_initialized = true;
    }
    
    // 创建mosquitto客户端实例
    m_mosquitto = mosquitto_new(m_client_id.c_str(), true, this);
    if (!m_mosquitto) {
        throw std::runtime_error("Failed to create mosquitto client");
    }
    
    // 设置回调函数
    mosquitto_connect_callback_set(m_mosquitto, onConnect);
    mosquitto_disconnect_callback_set(m_mosquitto, onDisconnect);
    mosquitto_message_callback_set(m_mosquitto, onMessage);
    mosquitto_subscribe_callback_set(m_mosquitto, onSubscribe);
    mosquitto_unsubscribe_callback_set(m_mosquitto, onUnsubscribe);
    mosquitto_publish_callback_set(m_mosquitto, onPublish);
    
    // 配置SSL/TLS
    if (m_ssl_config.enabled) {
        configureSsl(m_ssl_config);
    }
}

MqttClient::MqttClient(const std::string& client_id,
                       const std::string& host,
                       int port,
                       const AuthConfig& auth_config,
                       int keep_alive)
    : m_client_id(client_id)
    , m_host(host)
    , m_port(port)
    , m_keep_alive(keep_alive)
    , m_auth_config(auth_config)
    , m_connected(false)
    , m_running(false)
    , m_auto_reconnect(false)
    , m_retry_interval(5)
    , m_mosquitto(nullptr)
{
    // 初始化mosquitto库（线程安全）
    std::lock_guard<std::mutex> lock(s_init_mutex);
    if (!s_lib_initialized) {
        mosquitto_lib_init();
        s_lib_initialized = true;
    }
    
    // 创建mosquitto客户端实例
    m_mosquitto = mosquitto_new(m_client_id.c_str(), true, this);
    if (!m_mosquitto) {
        throw std::runtime_error("Failed to create mosquitto client");
    }
    
    // 设置回调函数
    mosquitto_connect_callback_set(m_mosquitto, onConnect);
    mosquitto_disconnect_callback_set(m_mosquitto, onDisconnect);
    mosquitto_message_callback_set(m_mosquitto, onMessage);
    mosquitto_subscribe_callback_set(m_mosquitto, onSubscribe);
    mosquitto_unsubscribe_callback_set(m_mosquitto, onUnsubscribe);
    mosquitto_publish_callback_set(m_mosquitto, onPublish);
    
    // 配置身份验证
    if (m_auth_config.enabled) {
        configureAuth(m_auth_config);
    }
}

MqttClient::MqttClient(const std::string& client_id,
                       const std::string& host,
                       int port,
                       const SslConfig& ssl_config,
                       const AuthConfig& auth_config,
                       int keep_alive)
    : m_client_id(client_id)
    , m_host(host)
    , m_port(port)
    , m_keep_alive(keep_alive)
    , m_ssl_config(ssl_config)
    , m_auth_config(auth_config)
    , m_connected(false)
    , m_running(false)
    , m_auto_reconnect(false)
    , m_retry_interval(5)
    , m_mosquitto(nullptr)
{
    // 初始化mosquitto库（线程安全）
    std::lock_guard<std::mutex> lock(s_init_mutex);
    if (!s_lib_initialized) {
        mosquitto_lib_init();
        s_lib_initialized = true;
    }
    
    // 创建mosquitto客户端实例
    m_mosquitto = mosquitto_new(m_client_id.c_str(), true, this);
    if (!m_mosquitto) {
        throw std::runtime_error("Failed to create mosquitto client");
    }
    
    // 设置回调函数
    mosquitto_connect_callback_set(m_mosquitto, onConnect);
    mosquitto_disconnect_callback_set(m_mosquitto, onDisconnect);
    mosquitto_message_callback_set(m_mosquitto, onMessage);
    mosquitto_subscribe_callback_set(m_mosquitto, onSubscribe);
    mosquitto_unsubscribe_callback_set(m_mosquitto, onUnsubscribe);
    mosquitto_publish_callback_set(m_mosquitto, onPublish);
    
    // 配置SSL/TLS
    if (m_ssl_config.enabled) {
        configureSsl(m_ssl_config);
    }
    
    // 配置身份验证
    if (m_auth_config.enabled) {
        configureAuth(m_auth_config);
    }
}

MqttClient::~MqttClient() {
    stop();
    
    if (m_mosquitto) {
        mosquitto_destroy(m_mosquitto);
    }
    
    // 清理mosquitto库
    std::lock_guard<std::mutex> lock(s_init_mutex);
    if (s_lib_initialized) {
        mosquitto_lib_cleanup();
        s_lib_initialized = false;
    }
}

bool MqttClient::connect() {
    if (!m_mosquitto) {
        return false;
    }
    
    int result = mosquitto_connect(m_mosquitto, m_host.c_str(), m_port, m_keep_alive);
    if (result != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to connect to MQTT broker: " << mosquitto_strerror(result) << std::endl;
        return false;
    }
    
    return true;
}

void MqttClient::disconnect() {
    if (m_mosquitto && m_connected) {
        mosquitto_disconnect(m_mosquitto);
    }
}

bool MqttClient::publish(const std::string& topic, 
                        const std::string& payload, 
                        int qos, 
                        bool retain) {
    if (!m_mosquitto || !m_connected) {
        return false;
    }
    
    int result = mosquitto_publish(m_mosquitto, nullptr, topic.c_str(), 
                                  payload.length(), payload.c_str(), qos, retain);
    
    return result == MOSQ_ERR_SUCCESS;
}

bool MqttClient::subscribe(const std::string& topic, int qos) {
    if (!m_mosquitto || !m_connected) {
        return false;
    }
    
    int result = mosquitto_subscribe(m_mosquitto, nullptr, topic.c_str(), qos);
    return result == MOSQ_ERR_SUCCESS;
}

bool MqttClient::unsubscribe(const std::string& topic) {
    if (!m_mosquitto || !m_connected) {
        return false;
    }
    
    int result = mosquitto_unsubscribe(m_mosquitto, nullptr, topic.c_str());
    return result == MOSQ_ERR_SUCCESS;
}

void MqttClient::setMessageCallback(MessageCallback callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_message_callback = callback;
}

void MqttClient::setConnectionCallback(ConnectionCallback callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connection_callback = callback;
}

void MqttClient::start() {
    if (m_running) {
        return;
    }
    
    m_running = true;
    
    // 启动消息循环线程，使用mosquitto_loop_forever
    m_loop_thread = std::thread([this]() {
        while (m_running) {
            int result = mosquitto_loop_forever(m_mosquitto, 1000, 1);
            if (result != MOSQ_ERR_SUCCESS) {
                std::cerr << "MQTT loop error: " << mosquitto_strerror(result) << std::endl;
                if (!m_running) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }
    });
    
    // 启动重连线程（如果启用自动重连）
    if (m_auto_reconnect) {
        m_reconnect_thread = std::thread(&MqttClient::reconnectLoop, this);
    }
}

void MqttClient::stop() {
    m_running = false;
    m_auto_reconnect = false;
    
    disconnect();
    
    // 等待线程结束
    if (m_loop_thread.joinable()) {
        m_loop_thread.join();
    }
    
    if (m_reconnect_thread.joinable()) {
        m_reconnect_thread.join();
    }
}

bool MqttClient::isConnected() const {
    return m_connected;
}

void MqttClient::setAutoReconnect(bool enable, int retry_interval) {
    m_auto_reconnect = enable;
    m_retry_interval = retry_interval;
    
    if (enable && m_running && !m_reconnect_thread.joinable()) {
        m_reconnect_thread = std::thread(&MqttClient::reconnectLoop, this);
    }
}

// 静态回调函数实现
void MqttClient::onConnect(struct mosquitto* mosq, void* userdata, int result) {
    MqttClient* client = static_cast<MqttClient*>(userdata);
    if (!client) return;
    
    client->m_connected = (result == 0);
    
    if (result == 0) {
        std::cout << "Connected to MQTT broker successfully" << std::endl;
    } else {
        std::cerr << "Failed to connect to MQTT broker: " << mosquitto_connack_string(result) << std::endl;
    }
    
    // 调用连接状态回调
    std::lock_guard<std::mutex> lock(client->m_mutex);
    if (client->m_connection_callback) {
        client->m_connection_callback(client->m_connected);
    }
}

void MqttClient::onDisconnect(struct mosquitto* mosq, void* userdata, int result) {
    MqttClient* client = static_cast<MqttClient*>(userdata);
    if (!client) return;
    
    client->m_connected = false;
    
    if (result == 0) {
        std::cout << "Disconnected from MQTT broker" << std::endl;
    } else {
        std::cerr << "Unexpected disconnection from MQTT broker: " << mosquitto_strerror(result) << std::endl;
    }
    
    // 调用连接状态回调
    std::lock_guard<std::mutex> lock(client->m_mutex);
    if (client->m_connection_callback) {
        client->m_connection_callback(false);
    }
}

void MqttClient::onMessage(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message) {
    MqttClient* client = static_cast<MqttClient*>(userdata);
    if (!client || !message) return;
    
    std::string topic(message->topic);
    std::string payload(static_cast<const char*>(message->payload), message->payloadlen);
    
    // 调用消息回调
    std::lock_guard<std::mutex> lock(client->m_mutex);
    if (client->m_message_callback) {
        client->m_message_callback(topic, payload);
    }
}

void MqttClient::onSubscribe(struct mosquitto* mosq, void* userdata, int mid, int qos_count, const int* granted_qos) {
    std::cout << "Subscription successful (mid: " << mid << ")" << std::endl;
}

void MqttClient::onUnsubscribe(struct mosquitto* mosq, void* userdata, int mid) {
    std::cout << "Unsubscription successful (mid: " << mid << ")" << std::endl;
}

void MqttClient::onPublish(struct mosquitto* mosq, void* userdata, int mid) {
    // 消息发布成功（可选择记录日志）
}

void MqttClient::reconnectLoop() {
    while (m_auto_reconnect && m_running) {
        if (!m_connected) {
            std::cout << "Attempting to reconnect to MQTT broker..." << std::endl;
            
            int result = mosquitto_reconnect(m_mosquitto);
            if (result == MOSQ_ERR_SUCCESS) {
                std::cout << "Reconnection initiated" << std::endl;
            } else {
                std::cerr << "Reconnection failed: " << mosquitto_strerror(result) << std::endl;
            }
        }
        
        // 等待重连间隔
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait_for(lock, std::chrono::seconds(m_retry_interval), [this]() {
            return !m_auto_reconnect || !m_running;
        });
    }
}

// 静态密码回调函数
static int password_callback(char *buf, int size, int rwflag, void *userdata) {
    const std::string* password = static_cast<const std::string*>(userdata);
    if (password && !password->empty()) {
        int len = std::min(size - 1, static_cast<int>(password->length()));
        std::strncpy(buf, password->c_str(), len);
        buf[len] = '\0';
        return len;
    }
    return 0;
}

bool MqttClient::configureSsl(const SslConfig& ssl_config) {
    if (!m_mosquitto) {
        return false;
    }
    
    m_ssl_config = ssl_config;
    
    if (!ssl_config.enabled) {
        return true;
    }
    
    int result;
    
    // 设置TLS选项
    if (!ssl_config.ca_file.empty()) {
        const char* cert_file = ssl_config.cert_file.empty() ? nullptr : ssl_config.cert_file.c_str();
        const char* key_file = ssl_config.key_file.empty() ? nullptr : ssl_config.key_file.c_str();
        
        // 设置密码回调函数（如果有密码）
        int (*pw_callback)(char *, int, int, void *) = nullptr;
        void* userdata = nullptr;
        if (!ssl_config.key_password.empty()) {
            pw_callback = password_callback;
            userdata = const_cast<std::string*>(&ssl_config.key_password);
        }
        
        result = mosquitto_tls_set(m_mosquitto, 
                                   ssl_config.ca_file.c_str(),
                                   nullptr,  // capath
                                   cert_file,
                                   key_file,
                                   pw_callback); // password callback function
        
        if (result != MOSQ_ERR_SUCCESS) {
            std::cerr << "Failed to set TLS certificates: " << mosquitto_strerror(result) << std::endl;
            return false;
        }
        
        // 设置用户数据用于密码回调
        if (userdata) {
            mosquitto_user_data_set(m_mosquitto, userdata);
        }
    }
    
    // 设置TLS选项（证书验证）
    result = mosquitto_tls_opts_set(m_mosquitto,
                                    0,          // 完全禁用证书验证
                                    nullptr,    // 不指定TLS版本，使用默认
                                    nullptr);   // 不指定加密套件，使用默认
    
    if (result != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to set TLS options: " << mosquitto_strerror(result) << std::endl;
        return false;
    }
    
    // 设置主机名验证
    result = mosquitto_tls_insecure_set(m_mosquitto, !ssl_config.verify_hostname);
    
    if (result != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to set TLS hostname verification: " << mosquitto_strerror(result) << std::endl;
        return false;
    }
    
    std::cout << "SSL/TLS configured successfully" << std::endl;
    return true;
}

const SslConfig& MqttClient::getSslConfig() const {
    return m_ssl_config;
}

bool MqttClient::configureAuth(const AuthConfig& auth_config) {
    if (!m_mosquitto) {
        std::cerr << "MQTT client not initialized" << std::endl;
        return false;
    }
    
    m_auth_config = auth_config;
    
    if (!auth_config.enabled) {
        std::cout << "Authentication disabled" << std::endl;
        return true;
    }
    
    if (auth_config.username.empty()) {
        std::cerr << "Username cannot be empty when authentication is enabled" << std::endl;
        return false;
    }
    
    std::cout << "Configuring MQTT authentication..." << std::endl;
    std::cout << "Username: " << auth_config.username << std::endl;
    std::cout << "Password: " << (auth_config.password.empty() ? "(empty)" : "(set)") << std::endl;
    
    // 设置用户名和密码
    int result = mosquitto_username_pw_set(m_mosquitto, 
                                          auth_config.username.c_str(),
                                          auth_config.password.empty() ? nullptr : auth_config.password.c_str());
    
    if (result != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to set username/password: " << mosquitto_strerror(result) << std::endl;
        return false;
    }
    
    std::cout << "Authentication configured successfully" << std::endl;
    return true;
}

const AuthConfig& MqttClient::getAuthConfig() const {
    return m_auth_config;
}