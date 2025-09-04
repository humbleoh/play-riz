#include <mosquitto.h>
#include <iostream>
#include <unistd.h>

int main() {
    // 初始化mosquitto库
    mosquitto_lib_init();
    
    // 创建客户端
    struct mosquitto *mosq = mosquitto_new("test_ssl_simple", true, nullptr);
    if (!mosq) {
        std::cerr << "Failed to create mosquitto client" << std::endl;
        return 1;
    }
    
    // 设置TLS证书
    int result = mosquitto_tls_set(mosq, 
                                   "certs/ca.crt",
                                   nullptr,  // capath
                                   "certs/client.crt",
                                   "certs/client.key",
                                   nullptr); // password callback
    
    if (result != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to set TLS certificates: " << mosquitto_strerror(result) << std::endl;
        mosquitto_destroy(mosq);
        return 1;
    }
    
    // 设置TLS选项
    result = mosquitto_tls_opts_set(mosq, 0, "tlsv1.2", nullptr);
    if (result != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to set TLS options: " << mosquitto_strerror(result) << std::endl;
        mosquitto_destroy(mosq);
        return 1;
    }
    
    // 禁用主机名验证
    result = mosquitto_tls_insecure_set(mosq, true);
    if (result != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to set TLS insecure: " << mosquitto_strerror(result) << std::endl;
        mosquitto_destroy(mosq);
        return 1;
    }
    
    std::cout << "SSL configuration completed successfully" << std::endl;
    
    // 尝试连接
    result = mosquitto_connect(mosq, "localhost", 8883, 60);
    if (result != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to connect: " << mosquitto_strerror(result) << std::endl;
        mosquitto_destroy(mosq);
        return 1;
    }
    
    std::cout << "Connection initiated, starting loop..." << std::endl;
    
    // 运行循环
    for (int i = 0; i < 10; i++) {
        result = mosquitto_loop(mosq, 1000, 1);
        if (result != MOSQ_ERR_SUCCESS) {
            std::cerr << "Loop error: " << mosquitto_strerror(result) << std::endl;
            break;
        }
        std::cout << "Loop iteration " << i << " completed" << std::endl;
        sleep(1);
    }
    
    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    
    return 0;
}