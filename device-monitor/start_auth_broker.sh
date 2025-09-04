#!/bin/bash

# 启动支持用户认证的Mosquitto broker

echo "Starting Mosquitto broker with authentication..."
echo "Configuration: mosquitto_auth.conf"
echo "Users:"
echo "  - testuser / testpass (for server/client testing)"
echo "  - device01 / devicepass (for device testing)"
echo ""
echo "Usage examples:"
echo "  Server: ./server --auth --username testuser --password testpass"
echo "  Device: ./device --id device01 --auth --username device01 --password devicepass"
echo ""
echo "Press Ctrl+C to stop the broker"
echo ""

# 创建日志目录
mkdir -p /tmp/mosquitto

# 启动Mosquitto
mosquitto -c mosquitto_auth.conf -v