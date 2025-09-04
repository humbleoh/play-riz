#include "server.h"
#include <iostream>
#include <signal.h>
#include <thread>
#include <chrono>
#include <json/json.h>

// 全局服务端实例
std::unique_ptr<Server> g_server;
bool g_running = true;

// 信号处理函数
void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    g_running = false;
    if (g_server) {
        g_server->stop();
    }
}

// 打印帮助信息
void printHelp() {
    std::cout << "Device Monitor Server" << std::endl;
    std::cout << "Usage: server [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help           Show this help message" << std::endl;
    std::cout << "  -i, --id <id>        Server ID (default: server1)" << std::endl;
    std::cout << "  -H, --host <host>    MQTT broker host (default: localhost)" << std::endl;
    std::cout << "  -p, --port <port>    MQTT broker port (default: 1883)" << std::endl;
    std::cout << "  -t, --timeout <sec>  Device timeout in seconds (default: 300)" << std::endl;
    std::cout << "  --ssl                Enable SSL/TLS connection" << std::endl;
    std::cout << "  --ca-file <path>     CA certificate file path" << std::endl;
    std::cout << "  --cert-file <path>   Client certificate file path" << std::endl;
    std::cout << "  --key-file <path>    Client private key file path" << std::endl;
    std::cout << "  --no-verify-peer     Disable peer certificate verification" << std::endl;
    std::cout << "  --no-verify-hostname Disable hostname verification" << std::endl;
    std::cout << "  --auth               Enable username/password authentication" << std::endl;
    std::cout << "  --username <user>    MQTT username for authentication" << std::endl;
    std::cout << "  --password <pass>    MQTT password for authentication" << std::endl;
}

// 交互式命令处理
void processInteractiveCommands(Server* server) {
    std::string input;
    std::cout << "\nServer started. Type 'help' for available commands." << std::endl;
    std::cout << "server> ";
    
    while (g_running && std::getline(std::cin, input)) {
        if (input.empty()) {
            std::cout << "server> ";
            continue;
        }
        
        std::istringstream iss(input);
        std::string command;
        iss >> command;
        
        if (command == "help") {
            std::cout << "Available commands:" << std::endl;
            std::cout << "  help                     - Show this help" << std::endl;
            std::cout << "  status                   - Show server status" << std::endl;
            std::cout << "  devices                  - List all devices" << std::endl;
            std::cout << "  online                   - List online devices" << std::endl;
            std::cout << "  device <id>              - Show device details" << std::endl;
            std::cout << "  send <device_id> <cmd>   - Send command to device" << std::endl;
            std::cout << "  refresh [device_id]      - Request device status update" << std::endl;
            std::cout << "  quit                     - Exit server" << std::endl;
        }
        else if (command == "status") {
            auto devices = server->getAllDeviceStatus();
            std::cout << "Server Status:" << std::endl;
            std::cout << "  Total devices: " << devices.size() << std::endl;
            
            int online_count = 0;
            for (const auto& pair : devices) {
                if (pair.second.status == "online") {
                    online_count++;
                }
            }
            std::cout << "  Online devices: " << online_count << std::endl;
        }
        else if (command == "devices") {
            auto devices = server->getAllDeviceStatus();
            std::cout << "All Devices:" << std::endl;
            for (const auto& pair : devices) {
                const auto& status = pair.second;
                auto now = std::chrono::system_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - status.last_seen).count();
                
                std::cout << "  " << pair.first << " - " << status.status 
                         << " (last seen " << elapsed << "s ago)" << std::endl;
            }
        }
        else if (command == "online") {
            auto online_devices = server->getOnlineDevices();
            std::cout << "Online Devices:" << std::endl;
            for (const auto& device_id : online_devices) {
                std::cout << "  " << device_id << std::endl;
            }
        }
        else if (command == "device") {
            std::string device_id;
            iss >> device_id;
            if (device_id.empty()) {
                std::cout << "Usage: device <device_id>" << std::endl;
            } else {
                auto status = server->getDeviceStatus(device_id);
                if (status) {
                    std::cout << "Device " << device_id << ":" << std::endl;
                    std::cout << "  Status: " << status->status << std::endl;
                    
                    auto now = std::chrono::system_clock::now();
                    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - status->last_seen).count();
                    std::cout << "  Last seen: " << elapsed << " seconds ago" << std::endl;
                    
                    if (!status->properties.empty()) {
                        std::cout << "  Properties:" << std::endl;
                        Json::StreamWriterBuilder builder;
                        builder["indentation"] = "    ";
                        std::string props_str = Json::writeString(builder, status->properties);
                        std::cout << props_str << std::endl;
                    }
                } else {
                    std::cout << "Device " << device_id << " not found" << std::endl;
                }
            }
        }
        else if (command == "send") {
            std::string device_id, cmd_type;
            iss >> device_id >> cmd_type;
            if (device_id.empty() || cmd_type.empty()) {
                std::cout << "Usage: send <device_id> <command_type> [parameters]" << std::endl;
            } else {
                Json::Value params;
                std::string param_line;
                std::getline(iss, param_line);
                if (!param_line.empty()) {
                    // 尝试解析参数为JSON
                    Json::CharReaderBuilder builder;
                    std::string errors;
                    std::istringstream param_stream(param_line);
                    if (!Json::parseFromStream(builder, param_stream, &params, &errors)) {
                        std::cout << "Invalid JSON parameters: " << errors << std::endl;
                        std::cout << "server> ";
                        continue;
                    }
                }
                
                std::string cmd_id = server->sendCommand(device_id, cmd_type, params);
                if (!cmd_id.empty()) {
                    std::cout << "Command sent with ID: " << cmd_id << std::endl;
                } else {
                    std::cout << "Failed to send command" << std::endl;
                }
            }
        }
        else if (command == "refresh") {
            std::string device_id;
            iss >> device_id;
            server->requestDeviceStatus(device_id);
            if (device_id.empty()) {
                std::cout << "Requested status update from all devices" << std::endl;
            } else {
                std::cout << "Requested status update from device " << device_id << std::endl;
            }
        }
        else if (command == "quit" || command == "exit") {
            g_running = false;
            break;
        }
        else {
            std::cout << "Unknown command: " << command << ". Type 'help' for available commands." << std::endl;
        }
        
        if (g_running) {
            std::cout << "server> ";
        }
    }
}

int main(int argc, char* argv[]) {
    // 默认参数
    std::string server_id = "server1";
    std::string mqtt_host = "localhost";
    int mqtt_port = 1883;
    int device_timeout = 300;
    
    // SSL配置参数
    bool ssl_enabled = false;
    std::string ca_file = "";
    std::string cert_file = "";
    std::string key_file = "";
    bool verify_peer = true;
    bool verify_hostname = true;
    
    // 身份验证配置参数
    bool auth_enabled = false;
    std::string username = "";
    std::string password = "";
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printHelp();
            return 0;
        }
        else if ((arg == "-i" || arg == "--id") && i + 1 < argc) {
            server_id = argv[++i];
        }
        else if ((arg == "-H" || arg == "--host") && i + 1 < argc) {
            mqtt_host = argv[++i];
        }
        else if ((arg == "-p" || arg == "--port") && i + 1 < argc) {
            mqtt_port = std::atoi(argv[++i]);
        }
        else if ((arg == "-t" || arg == "--timeout") && i + 1 < argc) {
            device_timeout = std::atoi(argv[++i]);
        }
        else if (arg == "--ssl") {
            ssl_enabled = true;
        }
        else if (arg == "--ca-file" && i + 1 < argc) {
            ca_file = argv[++i];
        }
        else if (arg == "--cert-file" && i + 1 < argc) {
            cert_file = argv[++i];
        }
        else if (arg == "--key-file" && i + 1 < argc) {
            key_file = argv[++i];
        }
        else if (arg == "--no-verify-peer") {
            verify_peer = false;
        }
        else if (arg == "--no-verify-hostname") {
            verify_hostname = false;
        }
        else if (arg == "--auth") {
            auth_enabled = true;
        }
        else if (arg == "--username" && i + 1 < argc) {
            username = argv[++i];
        }
        else if (arg == "--password" && i + 1 < argc) {
            password = argv[++i];
        }
        else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            printHelp();
            return 1;
        }
    }
    
    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    try {
        // 创建服务端实例
        if (ssl_enabled && auth_enabled) {
            // 创建SSL配置
            SslConfig ssl_config;
            ssl_config.enabled = true;
            ssl_config.ca_file = ca_file;
            ssl_config.cert_file = cert_file;
            ssl_config.key_file = key_file;
            ssl_config.verify_peer = verify_peer;
            ssl_config.verify_hostname = verify_hostname;
            ssl_config.tls_version = "tlsv1.2";
            ssl_config.ciphers = "HIGH:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!SRP:!CAMELLIA";
            
            // 创建身份验证配置
            AuthConfig auth_config;
            auth_config.enabled = true;
            auth_config.username = username;
            auth_config.password = password;
            
            g_server = std::make_unique<Server>(server_id, mqtt_host, mqtt_port, ssl_config, auth_config);
            
            std::cout << "SSL/TLS + Authentication enabled with configuration:" << std::endl;
            if (!ca_file.empty()) std::cout << "  CA file: " << ca_file << std::endl;
            if (!cert_file.empty()) std::cout << "  Cert file: " << cert_file << std::endl;
            if (!key_file.empty()) std::cout << "  Key file: " << key_file << std::endl;
            std::cout << "  Verify peer: " << (verify_peer ? "yes" : "no") << std::endl;
            std::cout << "  Verify hostname: " << (verify_hostname ? "yes" : "no") << std::endl;
            std::cout << "  Username: " << username << std::endl;
        } else if (ssl_enabled) {
            // 创建SSL配置
            SslConfig ssl_config;
            ssl_config.enabled = true;
            ssl_config.ca_file = ca_file;
            ssl_config.cert_file = cert_file;
            ssl_config.key_file = key_file;
            ssl_config.verify_peer = verify_peer;
            ssl_config.verify_hostname = verify_hostname;
            ssl_config.tls_version = "tlsv1.2";
            ssl_config.ciphers = "HIGH:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!SRP:!CAMELLIA";
            
            g_server = std::make_unique<Server>(server_id, mqtt_host, mqtt_port, ssl_config);
            
            std::cout << "SSL/TLS enabled with configuration:" << std::endl;
            if (!ca_file.empty()) std::cout << "  CA file: " << ca_file << std::endl;
            if (!cert_file.empty()) std::cout << "  Cert file: " << cert_file << std::endl;
            if (!key_file.empty()) std::cout << "  Key file: " << key_file << std::endl;
            std::cout << "  Verify peer: " << (verify_peer ? "yes" : "no") << std::endl;
            std::cout << "  Verify hostname: " << (verify_hostname ? "yes" : "no") << std::endl;
        } else if (auth_enabled) {
            // 创建身份验证配置
            AuthConfig auth_config;
            auth_config.enabled = true;
            auth_config.username = username;
            auth_config.password = password;
            
            g_server = std::make_unique<Server>(server_id, mqtt_host, mqtt_port, auth_config);
            
            std::cout << "Authentication enabled with configuration:" << std::endl;
            std::cout << "  Username: " << username << std::endl;
        } else {
            g_server = std::make_unique<Server>(server_id, mqtt_host, mqtt_port);
        }
        
        // 设置设备超时时间
        g_server->setDeviceTimeout(device_timeout);
        
        // 设置回调函数
        g_server->setDeviceStatusCallback([](const std::string& device_id, const DeviceStatus& status) {
            std::cout << "\nDevice " << device_id << " status changed to: " << status.status << std::endl;
            std::cout << "server> " << std::flush;
        });
        
        g_server->setCommandResponseCallback([](const std::string& command_id, const Json::Value& response) {
            std::cout << "\nReceived response for command " << command_id << ":" << std::endl;
            Json::StreamWriterBuilder builder;
            builder["indentation"] = "  ";
            std::string response_str = Json::writeString(builder, response);
            std::cout << response_str << std::endl;
            std::cout << "server> " << std::flush;
        });
        
        // 启动服务端
        if (!g_server->start()) {
            std::cerr << "Failed to start server" << std::endl;
            return 1;
        }
        
        std::cout << "Device Monitor Server started:" << std::endl;
        std::cout << "  Server ID: " << server_id << std::endl;
        std::cout << "  MQTT Broker: " << mqtt_host << ":" << mqtt_port << std::endl;
        std::cout << "  Device Timeout: " << device_timeout << " seconds" << std::endl;
        
        // 处理交互式命令
        processInteractiveCommands(g_server.get());
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Server shutdown complete." << std::endl;
    return 0;
}