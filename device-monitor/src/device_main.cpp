#include "device.h"
#include <iostream>
#include <signal.h>
#include <thread>
#include <chrono>
#include <random>
#include <json/json.h>

// 全局设备实例
std::unique_ptr<Device> g_device;
bool g_running = true;

// 信号处理函数
void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    g_running = false;
    if (g_device) {
        g_device->stop();
    }
}

// 打印帮助信息
void printHelp() {
    std::cout << "Device Monitor Client" << std::endl;
    std::cout << "Usage: device [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help              Show this help message" << std::endl;
    std::cout << "  -i, --id <id>           Device ID (required)" << std::endl;
    std::cout << "  -t, --type <type>       Device type (default: sensor)" << std::endl;
    std::cout << "  -H, --host <host>       MQTT broker host (default: localhost)" << std::endl;
    std::cout << "  -p, --port <port>       MQTT broker port (default: 1883)" << std::endl;
    std::cout << "  -s, --status <interval> Status report interval in seconds (default: 60)" << std::endl;
    std::cout << "  -b, --heartbeat <int>   Heartbeat interval in seconds (default: 30)" << std::endl;
    std::cout << "  --simulate              Enable simulation mode with random data" << std::endl;
}

// 模拟设备数据更新
void simulateDeviceData(Device* device) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> temp_dist(20.0, 35.0);
    std::uniform_real_distribution<> humidity_dist(30.0, 80.0);
    std::uniform_int_distribution<> status_dist(0, 100);
    
    while (g_running) {
        // 模拟温度传感器
        double temperature = temp_dist(gen);
        device->updateProperty("temperature", temperature);
        
        // 模拟湿度传感器
        double humidity = humidity_dist(gen);
        device->updateProperty("humidity", humidity);
        
        // 模拟设备状态
        int status_val = status_dist(gen);
        if (status_val > 95) {
            device->setDeviceStatus("error");
        } else if (status_val > 90) {
            device->setDeviceStatus("warning");
        } else {
            device->setDeviceStatus("online");
        }
        
        // 每10秒更新一次模拟数据
        for (int i = 0; i < 10 && g_running; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

// 交互式命令处理
void processInteractiveCommands(Device* device) {
    std::string input;
    std::cout << "\nDevice started. Type 'help' for available commands." << std::endl;
    std::cout << "device> ";
    
    while (g_running && std::getline(std::cin, input)) {
        if (input.empty()) {
            std::cout << "device> ";
            continue;
        }
        
        std::istringstream iss(input);
        std::string command;
        iss >> command;
        
        if (command == "help") {
            std::cout << "Available commands:" << std::endl;
            std::cout << "  help                        - Show this help" << std::endl;
            std::cout << "  status                      - Show device status" << std::endl;
            std::cout << "  properties                  - List all properties" << std::endl;
            std::cout << "  get <property>              - Get property value" << std::endl;
            std::cout << "  set <property> <value>      - Set property value" << std::endl;
            std::cout << "  report                      - Send status report" << std::endl;
            std::cout << "  setstatus <status>          - Set device status" << std::endl;
            std::cout << "  quit                        - Exit device" << std::endl;
        }
        else if (command == "status") {
            std::cout << "Device Status:" << std::endl;
            std::cout << "  ID: " << device->getDeviceId() << std::endl;
            std::cout << "  Type: " << device->getDeviceType() << std::endl;
            std::cout << "  Status: " << device->getDeviceStatus() << std::endl;
        }
        else if (command == "properties") {
            auto properties = device->getAllProperties();
            std::cout << "Device Properties:" << std::endl;
            for (const auto& pair : properties) {
                const auto& prop = pair.second;
                std::cout << "  " << pair.first << ": ";
                
                Json::StreamWriterBuilder builder;
                std::string value_str = Json::writeString(builder, prop.value);
                // 移除JSON字符串的引号（如果是字符串类型）
                if (value_str.front() == '"' && value_str.back() == '"') {
                    value_str = value_str.substr(1, value_str.length() - 2);
                }
                std::cout << value_str;
                
                if (!prop.unit.empty()) {
                    std::cout << " " << prop.unit;
                }
                if (prop.writable) {
                    std::cout << " (writable)";
                }
                std::cout << std::endl;
            }
        }
        else if (command == "get") {
            std::string property;
            iss >> property;
            if (property.empty()) {
                std::cout << "Usage: get <property_name>" << std::endl;
            } else {
                Json::Value value = device->getProperty(property);
                if (!value.isNull()) {
                    Json::StreamWriterBuilder builder;
                    std::string value_str = Json::writeString(builder, value);
                    std::cout << property << ": " << value_str << std::endl;
                } else {
                    std::cout << "Property " << property << " not found" << std::endl;
                }
            }
        }
        else if (command == "set") {
            std::string property, value_str;
            iss >> property >> value_str;
            if (property.empty() || value_str.empty()) {
                std::cout << "Usage: set <property_name> <value>" << std::endl;
            } else {
                // 尝试解析值
                Json::Value value;
                Json::CharReaderBuilder builder;
                std::string errors;
                std::istringstream value_stream(value_str);
                
                if (Json::parseFromStream(builder, value_stream, &value, &errors)) {
                    if (device->updateProperty(property, value)) {
                        std::cout << "Property " << property << " updated successfully" << std::endl;
                    } else {
                        std::cout << "Failed to update property " << property << " (not found or not writable)" << std::endl;
                    }
                } else {
                    // 如果JSON解析失败，尝试作为字符串
                    if (device->updateProperty(property, Json::Value(value_str))) {
                        std::cout << "Property " << property << " updated successfully" << std::endl;
                    } else {
                        std::cout << "Failed to update property " << property << " (not found or not writable)" << std::endl;
                    }
                }
            }
        }
        else if (command == "report") {
            device->reportStatus();
            std::cout << "Status report sent" << std::endl;
        }
        else if (command == "setstatus") {
            std::string status;
            iss >> status;
            if (status.empty()) {
                std::cout << "Usage: setstatus <status>" << std::endl;
                std::cout << "Valid statuses: online, offline, error, warning, maintenance" << std::endl;
            } else {
                device->setDeviceStatus(status);
                std::cout << "Device status set to: " << status << std::endl;
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
            std::cout << "device> ";
        }
    }
}

int main(int argc, char* argv[]) {
    // 默认参数
    std::string device_id;
    std::string device_type = "sensor";
    std::string mqtt_host = "localhost";
    int mqtt_port = 1883;
    int status_interval = 60;
    int heartbeat_interval = 30;
    bool simulate = false;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printHelp();
            return 0;
        }
        else if ((arg == "-i" || arg == "--id") && i + 1 < argc) {
            device_id = argv[++i];
        }
        else if ((arg == "-t" || arg == "--type") && i + 1 < argc) {
            device_type = argv[++i];
        }
        else if ((arg == "-H" || arg == "--host") && i + 1 < argc) {
            mqtt_host = argv[++i];
        }
        else if ((arg == "-p" || arg == "--port") && i + 1 < argc) {
            mqtt_port = std::atoi(argv[++i]);
        }
        else if ((arg == "-s" || arg == "--status") && i + 1 < argc) {
            status_interval = std::atoi(argv[++i]);
        }
        else if ((arg == "-b" || arg == "--heartbeat") && i + 1 < argc) {
            heartbeat_interval = std::atoi(argv[++i]);
        }
        else if (arg == "--simulate") {
            simulate = true;
        }
        else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            printHelp();
            return 1;
        }
    }
    
    // 检查必需参数
    if (device_id.empty()) {
        std::cerr << "Error: Device ID is required. Use -i or --id to specify." << std::endl;
        printHelp();
        return 1;
    }
    
    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    try {
        // 创建设备实例
        g_device = std::make_unique<Device>(device_id, device_type, mqtt_host, mqtt_port);
        
        // 设置上报间隔
        g_device->setStatusReportInterval(status_interval);
        g_device->setHeartbeatInterval(heartbeat_interval);
        
        // 设置初始属性
        g_device->setProperty("temperature", 25.0, "°C", true);
        g_device->setProperty("humidity", 50.0, "%", true);
        g_device->setProperty("firmware_version", "1.0.0", "", false);
        g_device->setProperty("model", device_type, "", false);
        
        // 注册自定义命令处理器
        g_device->registerCommandHandler("restart", [](const std::string& cmd_type, const Json::Value& params) {
            CommandResult result;
            result.success = true;
            result.result_data["message"] = "Device restart initiated";
            std::cout << "\nReceived restart command" << std::endl;
            std::cout << "device> " << std::flush;
            return result;
        });
        
        g_device->registerCommandHandler("get_info", [](const std::string& cmd_type, const Json::Value& params) {
            CommandResult result;
            result.success = true;
            result.result_data["device_info"] = "Sample IoT Device";
            result.result_data["capabilities"] = Json::Value(Json::arrayValue);
            result.result_data["capabilities"].append("temperature_sensing");
            result.result_data["capabilities"].append("humidity_sensing");
            result.result_data["capabilities"].append("remote_control");
            return result;
        });
        
        // 设置状态更新回调
        g_device->setStatusUpdateCallback([](const std::string& device_id) {
            // 可以在这里添加状态更新时的自定义逻辑
        });
        
        // 启动设备
        if (!g_device->start()) {
            std::cerr << "Failed to start device" << std::endl;
            return 1;
        }
        
        std::cout << "Device Monitor Client started:" << std::endl;
        std::cout << "  Device ID: " << device_id << std::endl;
        std::cout << "  Device Type: " << device_type << std::endl;
        std::cout << "  MQTT Broker: " << mqtt_host << ":" << mqtt_port << std::endl;
        std::cout << "  Status Interval: " << status_interval << " seconds" << std::endl;
        std::cout << "  Heartbeat Interval: " << heartbeat_interval << " seconds" << std::endl;
        
        // 启动模拟线程（如果启用）
        std::thread simulation_thread;
        if (simulate) {
            std::cout << "  Simulation Mode: Enabled" << std::endl;
            simulation_thread = std::thread(simulateDeviceData, g_device.get());
        }
        
        // 处理交互式命令
        processInteractiveCommands(g_device.get());
        
        // 等待模拟线程结束
        if (simulation_thread.joinable()) {
            simulation_thread.join();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Device shutdown complete." << std::endl;
    return 0;
}