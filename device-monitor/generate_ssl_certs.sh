#!/bin/bash

# 生成SSL证书用于Mosquitto MQTT Broker
# 包括CA证书、服务器证书和客户端证书

echo "生成SSL证书用于MQTT Broker..."

# 创建证书目录
mkdir -p certs
cd certs

# 清理旧证书
rm -f *.crt *.key *.csr *.srl

echo "1. 生成CA私钥..."
openssl genrsa -out ca.key 2048

echo "2. 生成CA证书..."
openssl req -new -x509 -days 365 -key ca.key -out ca.crt -subj "/C=CN/ST=Beijing/L=Beijing/O=TestOrg/OU=TestUnit/CN=TestCA"

echo "3. 生成服务器私钥..."
openssl genrsa -out server.key 2048

echo "4. 生成服务器证书签名请求..."
openssl req -new -key server.key -out server.csr -subj "/C=CN/ST=Beijing/L=Beijing/O=TestOrg/OU=TestUnit/CN=localhost"

echo "5. 生成服务器证书..."
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 365

echo "6. 生成客户端私钥..."
openssl genrsa -out client.key 2048

echo "7. 生成客户端证书签名请求..."
openssl req -new -key client.key -out client.csr -subj "/C=CN/ST=Beijing/L=Beijing/O=TestOrg/OU=TestUnit/CN=client"

echo "8. 生成客户端证书..."
openssl x509 -req -in client.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out client.crt -days 365

# 清理临时文件
rm -f *.csr *.srl

echo ""
echo "SSL证书生成完成！"
echo "生成的文件:"
ls -la *.crt *.key
echo ""
echo "现在可以启动SSL-enabled Mosquitto broker:"
echo "./start_ssl_broker.sh"