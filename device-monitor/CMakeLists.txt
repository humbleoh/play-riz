cmake_minimum_required(VERSION 3.16)
project(DeviceMonitor)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找必要的包
find_package(PkgConfig REQUIRED)
pkg_check_modules(MOSQUITTO REQUIRED libmosquitto)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${MOSQUITTO_INCLUDE_DIRS})
include_directories(${JSONCPP_INCLUDE_DIRS})

# 添加库搜索路径
link_directories(${MOSQUITTO_LIBRARY_DIRS})
link_directories(${JSONCPP_LIBRARY_DIRS})

# 设置源文件目录
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

# 创建静态库 - MQTT客户端基础类
add_library(mqtt_client STATIC
    ${SRC_DIR}/mqtt_client.cpp
)

# 链接MQTT库到基础客户端
target_link_libraries(mqtt_client ${MOSQUITTO_LIBRARIES})
target_compile_options(mqtt_client PRIVATE ${MOSQUITTO_CFLAGS_OTHER})

# 服务端可执行文件
add_executable(server
    ${SRC_DIR}/server.cpp
    ${SRC_DIR}/server_main.cpp
)

# 设备端可执行文件
add_executable(device
    ${SRC_DIR}/device.cpp
    ${SRC_DIR}/device_main.cpp
)

# 链接库
target_link_libraries(server mqtt_client ${JSONCPP_LIBRARIES} pthread)
target_link_libraries(device mqtt_client ${JSONCPP_LIBRARIES} pthread)

# 设置编译选项
target_compile_options(server PRIVATE ${JSONCPP_CFLAGS_OTHER})
target_compile_options(device PRIVATE ${JSONCPP_CFLAGS_OTHER})

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# 安装目标
install(TARGETS server device
    RUNTIME DESTINATION bin
)

# 创建目录结构
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/include)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/src)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/examples)