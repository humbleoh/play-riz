#include "server.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <json/json.h>

// MQTT主题定义
const std::string Server::TOPIC_DEVICE_STATUS = "device/+/status";
const std::string Server::TOPIC_DEVICE_COMMAND = "device/+/command";
const std::string Server::TOPIC_DEVICE_RESPONSE = "device/+/response";
const std::string Server::TOPIC_DEVICE_HEARTBEAT = "device/+/heartbeat";

Server::Server(const std::string& server_id, 
               const std::string& mqtt_host, 
               int mqtt_port)
    : m_server_id(server_id)
    , m_running(false)
    , m_device_timeout(300) // 默认5分钟超时
    , m_command_counter(0)
{
    // 创建MQTT客户端
    m_mqtt_client = std::make_unique<MqttClient>("server_" + server_id, mqtt_host, mqtt_port);
    
    // 设置消息回调
    m_mqtt_client->setMessageCallback(
        [this](const std::string& topic, const std::string& payload) {
            handleMessage(topic, payload);
        }
    );
    
    // 设置连接状态回调
    m_mqtt_client->setConnectionCallback(
        [this](bool connected) {
            if (connected) {
                std::cout << "Server MQTT client connected" << std::endl;
                // 重新订阅所有主题
                m_mqtt_client->subscribe(TOPIC_DEVICE_STATUS, 1);
                m_mqtt_client->subscribe(TOPIC_DEVICE_RESPONSE, 1);
                m_mqtt_client->subscribe(TOPIC_DEVICE_HEARTBEAT, 0);
            } else {
                std::cout << "Server MQTT client disconnected" << std::endl;
            }
        }
    );
    
    // 启用自动重连
    m_mqtt_client->setAutoReconnect(true, 5);
}

Server::~Server() {
    stop();
}

bool Server::start() {
    if (m_running) {
        return true;
    }
    
    // 连接MQTT服务器
    if (!m_mqtt_client->connect()) {
        std::cerr << "Failed to connect to MQTT broker" << std::endl;
        return false;
    }
    
    // 启动MQTT客户端
    m_mqtt_client->start();
    
    // 订阅设备主题
    m_mqtt_client->subscribe(TOPIC_DEVICE_STATUS, 1);
    m_mqtt_client->subscribe(TOPIC_DEVICE_RESPONSE, 1);
    m_mqtt_client->subscribe(TOPIC_DEVICE_HEARTBEAT, 0);
    
    m_running = true;
    
    // 启动设备超时检查线程
    m_timeout_check_thread = std::thread(&Server::deviceTimeoutCheck, this);
    
    std::cout << "Server started successfully" << std::endl;
    return true;
}

void Server::stop() {
    if (!m_running) {
        return;
    }
    
    m_running = false;
    
    // 停止MQTT客户端
    if (m_mqtt_client) {
        m_mqtt_client->stop();
    }
    
    // 等待超时检查线程结束
    if (m_timeout_check_thread.joinable()) {
        m_timeout_check_thread.join();
    }
    
    std::cout << "Server stopped" << std::endl;
}

std::string Server::sendCommand(const std::string& device_id, 
                               const std::string& command_type, 
                               const Json::Value& parameters) {
    if (!m_mqtt_client || !m_mqtt_client->isConnected()) {
        std::cerr << "MQTT client not connected" << std::endl;
        return "";
    }
    
    // 生成命令ID
    std::string command_id = generateCommandId();
    
    // 构建命令消息
    Json::Value command;
    command["command_id"] = command_id;
    command["command_type"] = command_type;
    command["parameters"] = parameters;
    command["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    // 序列化为JSON字符串
    Json::StreamWriterBuilder builder;
    std::string payload = Json::writeString(builder, command);
    
    // 发送命令
    std::string topic = "device/" + device_id + "/command";
    if (m_mqtt_client->publish(topic, payload, 1)) {
        // 记录待响应命令
        std::lock_guard<std::mutex> lock(m_commands_mutex);
        ControlCommand cmd;
        cmd.command_id = command_id;
        cmd.device_id = device_id;
        cmd.command_type = command_type;
        cmd.parameters = parameters;
        cmd.timestamp = std::chrono::system_clock::now();
        m_pending_commands[command_id] = cmd;
        
        std::cout << "Command sent to device " << device_id << ": " << command_type << std::endl;
        return command_id;
    } else {
        std::cerr << "Failed to send command to device " << device_id << std::endl;
        return "";
    }
}

std::shared_ptr<DeviceStatus> Server::getDeviceStatus(const std::string& device_id) const {
    std::lock_guard<std::mutex> lock(m_devices_mutex);
    auto it = m_devices.find(device_id);
    if (it != m_devices.end()) {
        return std::make_shared<DeviceStatus>(it->second);
    }
    return nullptr;
}

std::map<std::string, DeviceStatus> Server::getAllDeviceStatus() const {
    std::lock_guard<std::mutex> lock(m_devices_mutex);
    return m_devices;
}

std::vector<std::string> Server::getOnlineDevices() const {
    std::vector<std::string> online_devices;
    std::lock_guard<std::mutex> lock(m_devices_mutex);
    
    for (const auto& pair : m_devices) {
        if (pair.second.status == "online") {
            online_devices.push_back(pair.first);
        }
    }
    
    return online_devices;
}

void Server::setDeviceStatusCallback(DeviceStatusCallback callback) {
    m_device_status_callback = callback;
}

void Server::setCommandResponseCallback(CommandResponseCallback callback) {
    m_command_response_callback = callback;
}

void Server::setDeviceTimeout(int timeout_seconds) {
    m_device_timeout = timeout_seconds;
}

void Server::requestDeviceStatus(const std::string& device_id) {
    if (!m_mqtt_client || !m_mqtt_client->isConnected()) {
        return;
    }
    
    Json::Value request;
    request["type"] = "status_request";
    request["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    Json::StreamWriterBuilder builder;
    std::string payload = Json::writeString(builder, request);
    
    if (device_id.empty()) {
        // 请求所有设备状态
        m_mqtt_client->publish("server/status_request", payload, 0);
    } else {
        // 请求特定设备状态
        std::string topic = "device/" + device_id + "/status_request";
        m_mqtt_client->publish(topic, payload, 0);
    }
}

void Server::handleMessage(const std::string& topic, const std::string& payload) {
    try {
        // 解析设备ID
        std::string device_id = parseDeviceIdFromTopic(topic);
        if (device_id.empty()) {
            return;
        }
        
        // 根据主题类型处理消息
        if (topic.find("/status") != std::string::npos) {
            handleDeviceStatus(device_id, payload);
        } else if (topic.find("/response") != std::string::npos) {
            handleCommandResponse(device_id, payload);
        } else if (topic.find("/heartbeat") != std::string::npos) {
            handleDeviceHeartbeat(device_id, payload);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling message: " << e.what() << std::endl;
    }
}

void Server::handleDeviceStatus(const std::string& device_id, const std::string& payload) {
    try {
        Json::CharReaderBuilder builder;
        Json::Value root;
        std::string errors;
        std::istringstream stream(payload);
        
        if (!Json::parseFromStream(builder, stream, &root, &errors)) {
            std::cerr << "Failed to parse device status JSON: " << errors << std::endl;
            return;
        }
        
        std::lock_guard<std::mutex> lock(m_devices_mutex);
        DeviceStatus& status = m_devices[device_id];
        
        status.device_id = device_id;
        status.status = root.get("status", "unknown").asString();
        status.last_seen = std::chrono::system_clock::now();
        
        if (root.isMember("properties")) {
            status.properties = root["properties"];
        }
        
        std::cout << "Device " << device_id << " status updated: " << status.status << std::endl;
        
        // 调用状态变化回调
        if (m_device_status_callback) {
            m_device_status_callback(device_id, status);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling device status: " << e.what() << std::endl;
    }
}

void Server::handleCommandResponse(const std::string& device_id, const std::string& payload) {
    try {
        Json::CharReaderBuilder builder;
        Json::Value root;
        std::string errors;
        std::istringstream stream(payload);
        
        if (!Json::parseFromStream(builder, stream, &root, &errors)) {
            std::cerr << "Failed to parse command response JSON: " << errors << std::endl;
            return;
        }
        
        std::string command_id = root.get("command_id", "").asString();
        if (command_id.empty()) {
            return;
        }
        
        // 移除待响应命令
        {
            std::lock_guard<std::mutex> lock(m_commands_mutex);
            m_pending_commands.erase(command_id);
        }
        
        std::cout << "Received response for command " << command_id << " from device " << device_id << std::endl;
        
        // 调用命令响应回调
        if (m_command_response_callback) {
            m_command_response_callback(command_id, root);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling command response: " << e.what() << std::endl;
    }
}

void Server::handleDeviceHeartbeat(const std::string& device_id, const std::string& payload) {
    std::lock_guard<std::mutex> lock(m_devices_mutex);
    DeviceStatus& status = m_devices[device_id];
    
    status.device_id = device_id;
    status.last_seen = std::chrono::system_clock::now();
    
    // 如果设备之前是离线状态，现在收到心跳，更新为在线
    if (status.status == "offline" || status.status.empty()) {
        status.status = "online";
        std::cout << "Device " << device_id << " is now online (heartbeat received)" << std::endl;
        
        if (m_device_status_callback) {
            m_device_status_callback(device_id, status);
        }
    }
}

void Server::deviceTimeoutCheck() {
    while (m_running) {
        auto now = std::chrono::system_clock::now();
        std::vector<std::string> offline_devices;
        
        {
            std::lock_guard<std::mutex> lock(m_devices_mutex);
            for (auto& pair : m_devices) {
                auto& status = pair.second;
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - status.last_seen).count();
                
                if (elapsed > m_device_timeout && status.status != "offline") {
                    status.status = "offline";
                    offline_devices.push_back(pair.first);
                }
            }
        }
        
        // 通知设备离线
        for (const auto& device_id : offline_devices) {
            std::cout << "Device " << device_id << " is now offline (timeout)" << std::endl;
            if (m_device_status_callback) {
                auto status = getDeviceStatus(device_id);
                if (status) {
                    m_device_status_callback(device_id, *status);
                }
            }
        }
        
        // 每30秒检查一次
        std::this_thread::sleep_for(std::chrono::seconds(30));
    }
}

std::string Server::generateCommandId() {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    auto counter = m_command_counter.fetch_add(1);
    
    std::ostringstream oss;
    oss << "cmd_" << timestamp << "_" << counter;
    return oss.str();
}

std::string Server::parseDeviceIdFromTopic(const std::string& topic) {
    // 主题格式: device/{device_id}/{message_type}
    size_t first_slash = topic.find('/');
    if (first_slash == std::string::npos) {
        return "";
    }
    
    size_t second_slash = topic.find('/', first_slash + 1);
    if (second_slash == std::string::npos) {
        return "";
    }
    
    return topic.substr(first_slash + 1, second_slash - first_slash - 1);
}