#include "device.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <json/json.h>

Device::Device(const std::string& device_id, 
               const std::string& device_type,
               const std::string& mqtt_host, 
               int mqtt_port)
    : m_device_id(device_id)
    , m_device_type(device_type)
    , m_device_status("offline")
    , m_running(false)
    , m_status_report_interval(60)  // 默认60秒上报一次状态
    , m_heartbeat_interval(30)      // 默认30秒发送一次心跳
    , m_start_time(std::chrono::system_clock::now())
{
    // 创建MQTT客户端
    m_mqtt_client = std::make_unique<MqttClient>("device_" + device_id, mqtt_host, mqtt_port);
    
    // 设置MQTT主题
    m_topic_command = "device/" + device_id + "/command";
    m_topic_status = "device/" + device_id + "/status";
    m_topic_response = "device/" + device_id + "/response";
    m_topic_heartbeat = "device/" + device_id + "/heartbeat";
    m_topic_status_request = "device/" + device_id + "/status_request";
    
    // 设置消息回调
    m_mqtt_client->setMessageCallback(
        [this](const std::string& topic, const std::string& payload) {
            handleMessage(topic, payload);
        }
    );
    
    // 设置连接状态回调
    m_mqtt_client->setConnectionCallback(
        [this](bool connected) {
            handleConnectionChange(connected);
        }
    );
    
    // 启用自动重连
    m_mqtt_client->setAutoReconnect(true, 5);
    
    // 注册默认命令处理器
    registerCommandHandler("get_status", [this](const std::string& cmd_type, const Json::Value& params) {
        CommandResult result;
        result.success = true;
        result.result_data = buildStatusMessage();
        return result;
    });
    
    registerCommandHandler("set_property", [this](const std::string& cmd_type, const Json::Value& params) {
        CommandResult result;
        if (params.isMember("name") && params.isMember("value")) {
            std::string prop_name = params["name"].asString();
            Json::Value prop_value = params["value"];
            
            if (updateProperty(prop_name, prop_value)) {
                result.success = true;
                result.result_data["message"] = "Property updated successfully";
            } else {
                result.success = false;
                result.error_message = "Failed to update property or property not writable";
            }
        } else {
            result.success = false;
            result.error_message = "Missing required parameters: name, value";
        }
        return result;
    });
}

Device::~Device() {
    stop();
}

bool Device::start() {
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
    
    // 订阅命令主题和状态请求主题
    m_mqtt_client->subscribe(m_topic_command, 1);
    m_mqtt_client->subscribe(m_topic_status_request, 0);
    m_mqtt_client->subscribe("server/status_request", 0); // 订阅服务端广播的状态请求
    
    m_running = true;
    m_device_status = "online";
    
    // 启动状态上报线程
    m_status_report_thread = std::thread(&Device::statusReportLoop, this);
    
    // 启动心跳线程
    m_heartbeat_thread = std::thread(&Device::heartbeatLoop, this);
    
    // 立即上报一次状态
    reportStatus();
    
    std::cout << "Device " << m_device_id << " started successfully" << std::endl;
    return true;
}

void Device::stop() {
    if (!m_running) {
        return;
    }
    
    m_running = false;
    m_device_status = "offline";
    
    // 发送离线状态
    reportStatus();
    
    // 停止MQTT客户端
    if (m_mqtt_client) {
        m_mqtt_client->stop();
    }
    
    // 等待线程结束
    if (m_status_report_thread.joinable()) {
        m_status_report_thread.join();
    }
    
    if (m_heartbeat_thread.joinable()) {
        m_heartbeat_thread.join();
    }
    
    std::cout << "Device " << m_device_id << " stopped" << std::endl;
}

void Device::setProperty(const std::string& name, 
                        const Json::Value& value, 
                        const std::string& unit, 
                        bool writable) {
    std::lock_guard<std::mutex> lock(m_properties_mutex);
    DeviceProperty prop(name, value, unit, writable);
    m_properties[name] = prop;
}

Json::Value Device::getProperty(const std::string& name) const {
    std::lock_guard<std::mutex> lock(m_properties_mutex);
    auto it = m_properties.find(name);
    if (it != m_properties.end()) {
        return it->second.value;
    }
    return Json::Value::null;
}

bool Device::updateProperty(const std::string& name, const Json::Value& value) {
    std::lock_guard<std::mutex> lock(m_properties_mutex);
    auto it = m_properties.find(name);
    if (it != m_properties.end() && it->second.writable) {
        it->second.value = value;
        return true;
    }
    return false;
}

std::map<std::string, DeviceProperty> Device::getAllProperties() const {
    std::lock_guard<std::mutex> lock(m_properties_mutex);
    return m_properties;
}

void Device::registerCommandHandler(const std::string& command_type, CommandHandler handler) {
    std::lock_guard<std::mutex> lock(m_handlers_mutex);
    m_command_handlers[command_type] = handler;
}

void Device::setStatusUpdateCallback(StatusUpdateCallback callback) {
    m_status_update_callback = callback;
}

void Device::reportStatus() {
    if (!m_mqtt_client || !m_mqtt_client->isConnected()) {
        return;
    }
    
    Json::Value status_msg = buildStatusMessage();
    
    Json::StreamWriterBuilder builder;
    std::string payload = Json::writeString(builder, status_msg);
    
    m_mqtt_client->publish(m_topic_status, payload, 1);
    
    // 调用状态更新回调
    if (m_status_update_callback) {
        m_status_update_callback(m_device_id);
    }
}

void Device::setStatusReportInterval(int interval_seconds) {
    m_status_report_interval = interval_seconds;
}

void Device::setHeartbeatInterval(int interval_seconds) {
    m_heartbeat_interval = interval_seconds;
}

void Device::setDeviceStatus(const std::string& status) {
    m_device_status = status;
}

void Device::handleMessage(const std::string& topic, const std::string& payload) {
    try {
        if (topic == m_topic_command) {
            handleCommand(payload);
        } else if (topic == m_topic_status_request || topic == "server/status_request") {
            handleStatusRequest(payload);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling message: " << e.what() << std::endl;
    }
}

void Device::handleCommand(const std::string& payload) {
    try {
        Json::CharReaderBuilder builder;
        Json::Value root;
        std::string errors;
        std::istringstream stream(payload);
        
        if (!Json::parseFromStream(builder, stream, &root, &errors)) {
            std::cerr << "Failed to parse command JSON: " << errors << std::endl;
            return;
        }
        
        std::string command_id = root.get("command_id", "").asString();
        std::string command_type = root.get("command_type", "").asString();
        Json::Value parameters = root.get("parameters", Json::Value());
        
        if (command_id.empty() || command_type.empty()) {
            std::cerr << "Invalid command: missing command_id or command_type" << std::endl;
            return;
        }
        
        std::cout << "Received command: " << command_type << " (ID: " << command_id << ")" << std::endl;
        
        CommandResult result;
        result.command_id = command_id;
        
        // 查找并执行命令处理器
        {
            std::lock_guard<std::mutex> lock(m_handlers_mutex);
            auto it = m_command_handlers.find(command_type);
            if (it != m_command_handlers.end()) {
                result = it->second(command_type, parameters);
                result.command_id = command_id; // 确保命令ID正确
            } else {
                result.success = false;
                result.error_message = "Unknown command type: " + command_type;
            }
        }
        
        // 发送响应
        sendCommandResponse(result);
        
    } catch (const std::exception& e) {
        std::cerr << "Error handling command: " << e.what() << std::endl;
    }
}

void Device::handleStatusRequest(const std::string& payload) {
    // 收到状态请求，立即上报状态
    reportStatus();
}

void Device::sendCommandResponse(const CommandResult& result) {
    if (!m_mqtt_client || !m_mqtt_client->isConnected()) {
        return;
    }
    
    Json::Value response;
    response["command_id"] = result.command_id;
    response["success"] = result.success;
    response["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
        result.timestamp.time_since_epoch()).count();
    
    if (result.success) {
        response["result"] = result.result_data;
    } else {
        response["error"] = result.error_message;
    }
    
    Json::StreamWriterBuilder builder;
    std::string payload = Json::writeString(builder, response);
    
    m_mqtt_client->publish(m_topic_response, payload, 1);
    
    std::cout << "Response sent for command " << result.command_id << std::endl;
}

void Device::statusReportLoop() {
    while (m_running) {
        reportStatus();
        
        // 等待指定间隔时间
        for (int i = 0; i < m_status_report_interval && m_running; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

void Device::heartbeatLoop() {
    while (m_running) {
        if (m_mqtt_client && m_mqtt_client->isConnected()) {
            Json::Value heartbeat = buildHeartbeatMessage();
            
            Json::StreamWriterBuilder builder;
            std::string payload = Json::writeString(builder, heartbeat);
            
            m_mqtt_client->publish(m_topic_heartbeat, payload, 0);
        }
        
        // 等待指定间隔时间
        for (int i = 0; i < m_heartbeat_interval && m_running; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

Json::Value Device::buildStatusMessage() {
    Json::Value status;
    status["device_id"] = m_device_id;
    status["device_type"] = m_device_type;
    status["status"] = m_device_status;
    status["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    // 添加运行时间
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now() - m_start_time).count();
    status["uptime"] = static_cast<Json::Int64>(uptime);
    
    // 添加设备属性
    Json::Value properties;
    {
        std::lock_guard<std::mutex> lock(m_properties_mutex);
        for (const auto& pair : m_properties) {
            Json::Value prop;
            prop["value"] = pair.second.value;
            prop["unit"] = pair.second.unit;
            prop["writable"] = pair.second.writable;
            properties[pair.first] = prop;
        }
    }
    status["properties"] = properties;
    
    return status;
}

Json::Value Device::buildHeartbeatMessage() {
    Json::Value heartbeat;
    heartbeat["device_id"] = m_device_id;
    heartbeat["status"] = m_device_status;
    heartbeat["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    return heartbeat;
}

void Device::handleConnectionChange(bool connected) {
    if (connected) {
        std::cout << "Device " << m_device_id << " MQTT client connected" << std::endl;
        m_device_status = "online";
        
        // 重新订阅主题
        m_mqtt_client->subscribe(m_topic_command, 1);
        m_mqtt_client->subscribe(m_topic_status_request, 0);
        m_mqtt_client->subscribe("server/status_request", 0);
        
        // 立即上报状态
        reportStatus();
    } else {
        std::cout << "Device " << m_device_id << " MQTT client disconnected" << std::endl;
        // 注意：这里不立即设置为offline，因为可能会自动重连
    }
}