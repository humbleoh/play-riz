#!/bin/bash

# SSL证书生成脚本
# 用于生成测试用的自签名证书

set -e

# 配置参数
CERT_DIR="./certs"
CA_KEY="$CERT_DIR/ca.key"
CA_CERT="$CERT_DIR/ca.crt"
SERVER_KEY="$CERT_DIR/server.key"
SERVER_CERT="$CERT_DIR/server.crt"
CLIENT_KEY="$CERT_DIR/client.key"
CLIENT_CERT="$CERT_DIR/client.crt"

# 证书有效期（天）
VALIDITY_DAYS=365

# 证书信息
COUNTRY="CN"
STATE="Beijing"
CITY="Beijing"
ORGANIZATION="Device Monitor"
ORGANIZATIONAL_UNIT="IT Department"
COMMON_NAME="localhost"
EMAIL="admin@example.com"

echo "=== SSL证书生成脚本 ==="
echo "生成目录: $CERT_DIR"
echo "证书有效期: $VALIDITY_DAYS 天"
echo

# 创建证书目录
if [ ! -d "$CERT_DIR" ]; then
    mkdir -p "$CERT_DIR"
    echo "创建证书目录: $CERT_DIR"
fi

# 1. 生成CA私钥
echo "1. 生成CA私钥..."
openssl genrsa -out "$CA_KEY" 4096
chmod 600 "$CA_KEY"

# 2. 生成CA证书
echo "2. 生成CA证书..."
openssl req -new -x509 -days $VALIDITY_DAYS -key "$CA_KEY" -out "$CA_CERT" -subj "/C=$COUNTRY/ST=$STATE/L=$CITY/O=$ORGANIZATION/OU=$ORGANIZATIONAL_UNIT/CN=CA-$COMMON_NAME/emailAddress=$EMAIL"

# 3. 生成服务器私钥
echo "3. 生成服务器私钥..."
openssl genrsa -out "$SERVER_KEY" 4096
chmod 600 "$SERVER_KEY"

# 4. 生成服务器证书签名请求
echo "4. 生成服务器证书签名请求..."
openssl req -new -key "$SERVER_KEY" -out "$CERT_DIR/server.csr" -subj "/C=$COUNTRY/ST=$STATE/L=$CITY/O=$ORGANIZATION/OU=$ORGANIZATIONAL_UNIT/CN=$COMMON_NAME/emailAddress=$EMAIL"

# 5. 使用CA签名生成服务器证书
echo "5. 生成服务器证书..."
# 创建扩展配置文件
cat > "$CERT_DIR/server.ext" << EOF
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
subjectAltName = @alt_names

[alt_names]
DNS.1 = localhost
DNS.2 = *.localhost
IP.1 = 127.0.0.1
IP.2 = ::1
EOF

openssl x509 -req -in "$CERT_DIR/server.csr" -CA "$CA_CERT" -CAkey "$CA_KEY" -CAcreateserial -out "$SERVER_CERT" -days $VALIDITY_DAYS -extfile "$CERT_DIR/server.ext"

# 6. 生成客户端私钥
echo "6. 生成客户端私钥..."
openssl genrsa -out "$CLIENT_KEY" 4096
chmod 600 "$CLIENT_KEY"

# 7. 生成客户端证书签名请求
echo "7. 生成客户端证书签名请求..."
openssl req -new -key "$CLIENT_KEY" -out "$CERT_DIR/client.csr" -subj "/C=$COUNTRY/ST=$STATE/L=$CITY/O=$ORGANIZATION/OU=$ORGANIZATIONAL_UNIT/CN=client-$COMMON_NAME/emailAddress=$EMAIL"

# 8. 使用CA签名生成客户端证书
echo "8. 生成客户端证书..."
# 创建客户端扩展配置文件
cat > "$CERT_DIR/client.ext" << EOF
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
extendedKeyUsage = clientAuth
EOF

openssl x509 -req -in "$CERT_DIR/client.csr" -CA "$CA_CERT" -CAkey "$CA_KEY" -CAcreateserial -out "$CLIENT_CERT" -days $VALIDITY_DAYS -extfile "$CERT_DIR/client.ext"

# 9. 清理临时文件
echo "9. 清理临时文件..."
rm -f "$CERT_DIR/server.csr" "$CERT_DIR/client.csr" "$CERT_DIR/server.ext" "$CERT_DIR/client.ext" "$CERT_DIR/ca.srl"

# 10. 设置文件权限
echo "10. 设置文件权限..."
chmod 644 "$CA_CERT" "$SERVER_CERT" "$CLIENT_CERT"
chmod 600 "$CA_KEY" "$SERVER_KEY" "$CLIENT_KEY"

echo
echo "=== 证书生成完成 ==="
echo "生成的文件:"
echo "  CA证书: $CA_CERT"
echo "  CA私钥: $CA_KEY"
echo "  服务器证书: $SERVER_CERT"
echo "  服务器私钥: $SERVER_KEY"
echo "  客户端证书: $CLIENT_CERT"
echo "  客户端私钥: $CLIENT_KEY"
echo
echo "证书信息:"
openssl x509 -in "$CA_CERT" -text -noout | grep -E "Subject:|Not Before:|Not After:"
echo
echo "使用说明:"
echo "1. 将CA证书添加到系统信任列表或MQTT broker配置中"
echo "2. 配置MQTT broker使用服务器证书和私钥"
echo "3. 客户端使用CA证书验证服务器，或使用客户端证书进行双向认证"
echo "4. 在配置文件中设置相应的证书路径"
echo
echo "注意: 这些是自签名证书，仅用于测试环境！"
echo "生产环境请使用由受信任CA签发的证书。"