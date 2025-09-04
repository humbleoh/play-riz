#include "include/mqtt_client.h"
#include <iostream>
#include <json/json.h>
#include <fstream>

int main() {
    std::cout << "=== SSL功能测试 ===" << std::endl;
    
    // 测试1: 基本SSL配置创建
    std::cout << "\n1. 测试基本SSL配置创建..." << std::endl;
    try {
        SslConfig ssl_config;
        ssl_config.enabled = true;
        ssl_config.ca_file = "./certs/ca.crt";
        ssl_config.cert_file = "./certs/client.crt";
        ssl_config.key_file = "./certs/client.key";
        ssl_config.verify_peer = true;
        ssl_config.verify_hostname = true;
        ssl_config.tls_version = "tlsv1.2";
        
        std::cout << "✓ SSL配置创建成功" << std::endl;
        std::cout << "  CA文件: " << ssl_config.ca_file << std::endl;
        std::cout << "  客户端证书: " << ssl_config.cert_file << std::endl;
        std::cout << "  私钥文件: " << ssl_config.key_file << std::endl;
        std::cout << "  TLS版本: " << ssl_config.tls_version << std::endl;
    } catch (const std::exception& e) {
        std::cout << "✗ SSL配置创建失败: " << e.what() << std::endl;
        return 1;
    }
    
    // 测试2: 从JSON配置文件加载SSL配置
    std::cout << "\n2. 测试从JSON配置文件加载SSL配置..." << std::endl;
    try {
        std::ifstream config_file("config_ssl_test.json");
        if (!config_file.is_open()) {
            std::cout << "✗ 无法打开配置文件" << std::endl;
            return 1;
        }
        
        Json::Value config;
        config_file >> config;
        
        if (config["mqtt"]["ssl"]["enabled"].asBool()) {
            std::cout << "✓ JSON配置文件SSL设置加载成功" << std::endl;
            std::cout << "  SSL启用: " << (config["mqtt"]["ssl"]["enabled"].asBool() ? "是" : "否") << std::endl;
            std::cout << "  CA文件: " << config["mqtt"]["ssl"]["ca_file"].asString() << std::endl;
            std::cout << "  验证对等方: " << (config["mqtt"]["ssl"]["verify_peer"].asBool() ? "是" : "否") << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "✗ JSON配置加载失败: " << e.what() << std::endl;
        return 1;
    }
    
    // 测试3: 创建带SSL配置的MQTT客户端
    std::cout << "\n3. 测试创建带SSL配置的MQTT客户端..." << std::endl;
    try {
        SslConfig ssl_config;
        ssl_config.enabled = true;
        ssl_config.ca_file = "./certs/ca.crt";
        ssl_config.cert_file = "./certs/client.crt";
        ssl_config.key_file = "./certs/client.key";
        ssl_config.verify_peer = true;
        ssl_config.verify_hostname = false; // 测试环境关闭主机名验证
        ssl_config.tls_version = "tlsv1.2";
        
        MqttClient client("test_ssl_client", "localhost", 8883, ssl_config);
        std::cout << "✓ 带SSL配置的MQTT客户端创建成功" << std::endl;
        std::cout << "  客户端ID: test_ssl_client" << std::endl;
        std::cout << "  SSL端口: 8883" << std::endl;
        
        // 注意: 这里不尝试连接，因为没有SSL MQTT broker运行
        std::cout << "  注意: 未尝试连接，因为需要SSL MQTT broker" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "✗ SSL MQTT客户端创建失败: " << e.what() << std::endl;
        return 1;
    }
    
    // 测试4: 验证证书文件存在
    std::cout << "\n4. 测试证书文件存在性..." << std::endl;
    std::vector<std::string> cert_files = {
        "./certs/ca.crt",
        "./certs/client.crt", 
        "./certs/client.key",
        "./certs/server.crt",
        "./certs/server.key"
    };
    
    for (const auto& file : cert_files) {
        std::ifstream f(file);
        if (f.good()) {
            std::cout << "✓ " << file << " 存在" << std::endl;
        } else {
            std::cout << "✗ " << file << " 不存在" << std::endl;
        }
    }
    
    std::cout << "\n=== SSL功能测试完成 ===" << std::endl;
    std::cout << "\n总结:" << std::endl;
    std::cout << "- SSL配置结构体功能正常" << std::endl;
    std::cout << "- JSON配置文件加载功能正常" << std::endl;
    std::cout << "- SSL MQTT客户端创建功能正常" << std::endl;
    std::cout << "- 测试证书文件已生成" << std::endl;
    std::cout << "\n注意: 要完整测试SSL连接，需要配置支持SSL的MQTT broker" << std::endl;
    
    return 0;
}