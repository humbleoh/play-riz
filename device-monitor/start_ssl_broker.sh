#!/bin/bash

# 启动支持SSL的Mosquitto MQTT Broker
# 使用项目中的SSL证书和配置文件

echo "启动SSL-enabled Mosquitto MQTT Broker..."
echo "配置文件: mosquitto_ssl.conf"
echo "SSL端口: 8883"
echo "非SSL端口: 1883 (仅本地)"
echo "证书目录: certs/"
echo ""

# 检查证书文件是否存在
if [ ! -f "certs/ca.crt" ] || [ ! -f "certs/server.crt" ] || [ ! -f "certs/server.key" ]; then
    echo "错误: SSL证书文件不存在！"
    echo "请先运行: ./scripts/generate_ssl_certs.sh"
    exit 1
fi

# 创建临时目录
mkdir -p /tmp/mosquitto

# 启动Mosquitto broker
echo "正在启动Mosquitto broker..."
mosquitto -c mosquitto_ssl.conf -v