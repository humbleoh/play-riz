#include "../include/server.h"
#include "../include/device.h"
#include <iostream>
#include <json/json.h>
#include <fstream>
#include <thread>
#include <chrono>

/**
 * SSL/TLS 使用示例
 * 演示如何配置和使用SSL/TLS加密连接
 */

// 从配置文件加载SSL配置
SslConfig loadSslConfigFromFile(const std::string& config_file) {
    SslConfig ssl_config;
    
    std::ifstream file(config_file);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << config_file << std::endl;
        return ssl_config;
    }
    
    Json::Value root;
    Json::Reader reader;
    
    if (!reader.parse(file, root)) {
        std::cerr << "Failed to parse config file: " << reader.getFormattedErrorMessages() << std::endl;
        return ssl_config;
    }
    
    if (root.isMember("mqtt") && root["mqtt"].isMember("ssl")) {
        Json::Value ssl = root["mqtt"]["ssl"];
        
        ssl_config.enabled = ssl.get("enabled", false).asBool();
        ssl_config.ca_file = ssl.get("ca_file", "").asString();
        ssl_config.cert_file = ssl.get("cert_file", "").asString();
        ssl_config.key_file = ssl.get("key_file", "").asString();
        ssl_config.key_password = ssl.get("key_password", "").asString();
        ssl_config.verify_peer = ssl.get("verify_peer", true).asBool();
        ssl_config.verify_hostname = ssl.get("verify_hostname", true).asBool();
        ssl_config.ciphers = ssl.get("ciphers", "").asString();
        ssl_config.tls_version = ssl.get("tls_version", "tlsv1.2").asString();
    }
    
    return ssl_config;
}

// 创建基本的SSL配置（用于测试）
SslConfig createBasicSslConfig() {
    SslConfig ssl_config;
    ssl_config.enabled = true;
    ssl_config.ca_file = "/etc/ssl/certs/ca-certificates.crt";  // 系统CA证书
    ssl_config.verify_peer = true;
    ssl_config.verify_hostname = true;
    ssl_config.tls_version = "tlsv1.2";
    return ssl_config;
}

// 创建自签名证书的SSL配置（用于开发测试）
SslConfig createSelfSignedSslConfig() {
    SslConfig ssl_config;
    ssl_config.enabled = true;
    ssl_config.ca_file = "./certs/ca.crt";
    ssl_config.cert_file = "./certs/client.crt";
    ssl_config.key_file = "./certs/client.key";
    ssl_config.verify_peer = false;  // 自签名证书通常不验证
    ssl_config.verify_hostname = false;
    ssl_config.tls_version = "tlsv1.2";
    return ssl_config;
}

void runServerExample() {
    std::cout << "=== SSL Server Example ===" << std::endl;
    
    // 方式1: 从配置文件加载SSL配置
    SslConfig ssl_config = loadSslConfigFromFile("config_ssl.example.json");
    
    // 方式2: 手动创建SSL配置
    if (!ssl_config.enabled) {
        ssl_config = createBasicSslConfig();
        std::cout << "Using basic SSL configuration" << std::endl;
    }
    
    try {
        // 创建支持SSL的服务端
        Server server("ssl_server", "mqtt.example.com", 8883, ssl_config);
        
        // 设置设备状态回调
        server.setDeviceStatusCallback([](const std::string& device_id, const DeviceStatus& status) {
            std::cout << "Device " << device_id << " status: " << status.status << std::endl;
        });
        
        // 启动服务端
        if (server.start()) {
            std::cout << "SSL Server started successfully" << std::endl;
            
            // 运行一段时间
            std::this_thread::sleep_for(std::chrono::seconds(30));
            
            server.stop();
            std::cout << "SSL Server stopped" << std::endl;
        } else {
            std::cerr << "Failed to start SSL server" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
}

void runDeviceExample() {
    std::cout << "=== SSL Device Example ===" << std::endl;
    
    // 创建SSL配置
    SslConfig ssl_config = createBasicSslConfig();
    
    try {
        // 创建支持SSL的设备
        Device device("ssl_device_001", "temperature_sensor", "mqtt.example.com", 8883, ssl_config);
        
        // 设置设备属性
        device.setProperty("temperature", 25.5, "°C", true);
        device.setProperty("humidity", 60.0, "%", true);
        device.setProperty("location", "Room A", "", false);
        
        // 注册自定义命令处理器
        device.registerCommandHandler("read_sensor", [&device](const std::string& cmd_type, const Json::Value& params) {
            CommandResult result;
            result.success = true;
            result.result_data["temperature"] = device.getProperty("temperature");
            result.result_data["humidity"] = device.getProperty("humidity");
            return result;
        });
        
        // 启动设备
        if (device.start()) {
            std::cout << "SSL Device started successfully" << std::endl;
            
            // 模拟传感器数据变化
            for (int i = 0; i < 10; ++i) {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                
                // 更新传感器数据
                double temp = 20.0 + (rand() % 20);
                double hum = 40.0 + (rand() % 40);
                
                device.updateProperty("temperature", temp);
                device.updateProperty("humidity", hum);
                
                std::cout << "Updated: temp=" << temp << "°C, hum=" << hum << "%" << std::endl;
            }
            
            device.stop();
            std::cout << "SSL Device stopped" << std::endl;
        } else {
            std::cerr << "Failed to start SSL device" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Device error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " [server|device]" << std::endl;
        std::cout << "Examples:" << std::endl;
        std::cout << "  " << argv[0] << " server   # Run SSL server example" << std::endl;
        std::cout << "  " << argv[0] << " device   # Run SSL device example" << std::endl;
        return 1;
    }
    
    std::string mode = argv[1];
    
    if (mode == "server") {
        runServerExample();
    } else if (mode == "device") {
        runDeviceExample();
    } else {
        std::cerr << "Invalid mode: " << mode << std::endl;
        return 1;
    }
    
    return 0;
}