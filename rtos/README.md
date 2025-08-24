# STM32 BluePill 简单 RTOS 教程

这是一个基于 STM32F103C8T6 (BluePill) 开发板的简单实时操作系统 (RTOS) 实现教程。本项目从零开始构建一个功能完整的 RTOS，包含任务调度、时间管理、同步机制等核心功能。

## 🎯 项目特性

- **抢占式多任务调度**: 支持基于优先级的任务调度
- **时间片轮转**: 同优先级任务的公平调度
- **信号量同步**: 提供任务间同步和互斥机制
- **动态任务管理**: 支持任务的创建、删除、挂起和恢复
- **系统时钟管理**: 基于 SysTick 的精确时间管理
- **硬件抽象层**: 针对 STM32F103C8T6 的 HAL 实现

## 📁 项目结构

```
rtos/
├── include/
│   ├── rtos.h          # RTOS 核心头文件
│   └── stm32f103.h     # STM32F103 硬件定义
├── src/
│   ├── main.c          # 主程序和示例应用
│   ├── rtos.c          # RTOS 核心功能
│   ├── scheduler.c     # 任务调度器
│   ├── task.c          # 任务管理
│   ├── semaphore.c     # 信号量实现
│   ├── systick.c       # 系统时钟管理
│   ├── stm32f103.c     # 硬件抽象层
│   ├── startup.c       # 启动代码和中断向量表
│   └── context_switch.s # 上下文切换汇编代码
├── Makefile            # 构建脚本
├── simple_rtos.ld      # 链接脚本
└── README.md           # 本文档
```

## 🛠️ 开发环境准备

### 必需工具

1. **ARM GCC 工具链**
   ```bash
   # macOS (使用 Homebrew)
   brew install --cask gcc-arm-embedded
   
   # Ubuntu/Debian
   sudo apt-get install gcc-arm-none-eabi
   
   # Windows
   # 下载并安装 GNU Arm Embedded Toolchain
   ```

2. **烧录工具** (选择其一)
   - **ST-Link**: `brew install stlink` (macOS) 或 `sudo apt-get install stlink-tools` (Linux)
   - **OpenOCD**: `brew install openocd` (macOS) 或 `sudo apt-get install openocd` (Linux)

3. **调试工具** (可选)
   - **GDB**: 通常随 ARM GCC 工具链一起安装

### 硬件要求

- STM32F103C8T6 BluePill 开发板
- ST-Link V2 调试器 (或兼容设备)
- USB 转串口模块 (可选，用于调试输出)

## 🚀 快速开始

### 1. 克隆项目

```bash
git clone <repository-url>
cd rtos
```

### 2. 编译项目

```bash
make all
```

编译成功后，会在 `build/` 目录下生成以下文件：
- `simple_rtos.elf` - ELF 格式的可执行文件
- `simple_rtos.hex` - Intel HEX 格式文件
- `simple_rtos.bin` - 二进制文件
- `simple_rtos.map` - 内存映射文件

### 3. 烧录到开发板

使用 ST-Link:
```bash
make flash
```

使用 OpenOCD:
```bash
make flash-openocd
```

### 4. 观察运行效果

烧录成功后，BluePill 板载 LED (PC13) 会按照以下模式闪烁：
- 正常闪烁：LED 闪烁任务
- 快速闪烁：快速闪烁任务
- 长亮：系统监控任务
- 短促闪烁：测试任务

## 📚 RTOS 核心概念

### 任务 (Task)

任务是 RTOS 中的基本执行单元。每个任务都有：
- **任务控制块 (TCB)**: 存储任务状态信息
- **独立栈空间**: 每个任务有 256 字节的栈空间
- **优先级**: 支持 4 个优先级等级
- **状态**: 就绪、运行、阻塞、挂起

```c
// 创建任务示例
uint32_t task_id = rtos_create_task(my_task_function, PRIORITY_NORMAL);

void my_task_function(void) {
    while (1) {
        // 任务代码
        led_toggle();
        rtos_delay(1000);  // 延时 1 秒
    }
}
```

### 调度器 (Scheduler)

调度器负责决定哪个任务应该运行：
- **抢占式调度**: 高优先级任务可以抢占低优先级任务
- **时间片轮转**: 同优先级任务轮流执行
- **空闲任务**: 当没有其他任务就绪时运行

### 信号量 (Semaphore)

信号量用于任务间的同步和互斥：

```c
// 创建信号量
semaphore_t my_semaphore;
semaphore_init(&my_semaphore, 1);  // 互斥信号量

// 在任务中使用
void task1(void) {
    while (1) {
        semaphore_wait(&my_semaphore);    // 获取信号量
        // 临界区代码
        semaphore_signal(&my_semaphore);  // 释放信号量
        rtos_delay(100);
    }
}
```

## 🔧 API 参考

### 核心函数

```c
// RTOS 初始化和启动
void rtos_init(void);                    // 初始化 RTOS
void rtos_start(void);                   // 启动调度器

// 任务管理
uint32_t rtos_create_task(void (*func)(void), task_priority_t priority);
void rtos_delete_task(uint32_t task_id);
void rtos_suspend_task(uint32_t task_id);
void rtos_resume_task(uint32_t task_id);
void rtos_yield(void);                   // 主动让出 CPU
void rtos_delay(uint32_t ms);            // 延时

// 信号量
void semaphore_init(semaphore_t *sem, int32_t count);
void semaphore_wait(semaphore_t *sem);
void semaphore_signal(semaphore_t *sem);
bool semaphore_try_wait(semaphore_t *sem);

// 系统信息
uint32_t rtos_get_uptime_ms(void);       // 获取系统运行时间(毫秒)
uint32_t rtos_get_task_count(void);      // 获取任务数量
const char* rtos_get_version(void);      // 获取版本信息
```

### 优先级定义

```c
typedef enum {
    PRIORITY_LOW = 0,      // 低优先级
    PRIORITY_NORMAL = 1,   // 普通优先级
    PRIORITY_HIGH = 2,     // 高优先级
    PRIORITY_CRITICAL = 3  // 关键优先级
} task_priority_t;
```

## 🎓 学习步骤

### 第一步：理解基础概念
1. 阅读 `include/rtos.h` 了解数据结构
2. 查看 `src/main.c` 中的示例应用
3. 理解任务、调度器、信号量的概念

### 第二步：分析核心实现
1. **启动过程**: `src/startup.c` - 中断向量表和启动代码
2. **硬件抽象**: `src/stm32f103.c` - GPIO 和系统时钟
3. **时间管理**: `src/systick.c` - SysTick 配置和中断处理

### 第三步：深入调度机制
1. **任务管理**: `src/task.c` - 任务创建、删除、栈初始化
2. **调度算法**: `src/scheduler.c` - 任务选择和调度逻辑
3. **上下文切换**: `src/context_switch.s` - 汇编级别的任务切换

### 第四步：同步机制
1. **信号量实现**: `src/semaphore.c` - P/V 操作和等待队列
2. **临界区保护**: 中断禁用/启用机制

### 第五步：实践和扩展
1. 修改示例程序，添加新的任务
2. 实验不同的优先级配置
3. 尝试添加新的同步原语（如互斥锁、事件标志）

## 🔍 调试技巧

### 使用 GDB 调试

```bash
# 启动 OpenOCD (另一个终端)
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg

# 启动 GDB 调试
make debug

# 在 GDB 中连接到目标
(gdb) target remote localhost:3333
(gdb) load
(gdb) monitor reset halt
(gdb) continue
```

### 常用 GDB 命令

```bash
(gdb) break main              # 在 main 函数设置断点
(gdb) break scheduler_run     # 在调度器函数设置断点
(gdb) info tasks              # 查看任务信息 (如果实现了)
(gdb) print current_task      # 打印当前任务指针
(gdb) x/16x 0x20000000       # 查看内存内容
```

### LED 调试模式

由于没有串口输出，可以使用 LED 闪烁模式来调试：
- 1 次闪烁：初始化完成
- 2 次闪烁：任务创建成功
- 3 次闪烁：调度器启动
- 长亮：系统错误

## ⚠️ 常见问题

### 编译错误

**问题**: `arm-none-eabi-gcc: command not found`
**解决**: 确保 ARM GCC 工具链已正确安装并添加到 PATH

**问题**: 链接错误 `undefined reference to 'xxx'`
**解决**: 检查函数声明和实现是否匹配，确保所有源文件都被编译

### 烧录问题

**问题**: `Error: libusb_open() failed with LIBUSB_ERROR_ACCESS`
**解决**: Linux 下需要配置 udev 规则或使用 sudo

**问题**: 找不到 ST-Link 设备
**解决**: 检查 USB 连接，确保 ST-Link 驱动已安装

### 运行时问题

**问题**: LED 不闪烁
**解决**: 检查时钟配置，确保 SysTick 正常工作

**问题**: 系统死锁
**解决**: 检查信号量使用，避免循环等待

## 🚧 扩展建议

### 功能扩展
1. **串口通信**: 添加 UART 支持，实现调试输出
2. **定时器**: 实现软件定时器功能
3. **消息队列**: 添加任务间消息传递机制
4. **内存管理**: 实现动态内存分配
5. **文件系统**: 添加简单的文件系统支持

### 性能优化
1. **调度算法**: 实现更高效的调度算法
2. **中断处理**: 优化中断响应时间
3. **内存使用**: 减少内存占用，支持更多任务

### 移植适配
1. **其他 STM32**: 适配到其他 STM32 系列
2. **其他 ARM**: 移植到其他 ARM Cortex-M 处理器
3. **RISC-V**: 移植到 RISC-V 架构

## 📖 参考资料

- [ARM Cortex-M3 技术参考手册](https://developer.arm.com/documentation/ddi0337/latest/)
- [STM32F103 数据手册](https://www.st.com/resource/en/datasheet/stm32f103c8.pdf)
- [FreeRTOS 官方文档](https://www.freertos.org/Documentation/RTOS_book.html)
- [μC/OS-II 实时内核](https://www.micrium.com/books/ucosii/)

## 📄 许可证

本项目采用 MIT 许可证，详见 LICENSE 文件。

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

---

**作者**: RTOS 学习者  
**版本**: v1.0  
**更新时间**: 2024年

祝你学习愉快！🎉