# STM32F303CCT6 LED闪烁项目 Makefile

# 项目名称
PROJECT = led_blink

# 目标芯片
TARGET = stm32f303cct6

# 工具链前缀
PREFIX = arm-none-eabi-

# 编译器和工具
CC = $(PREFIX)gcc
AS = $(PREFIX)as
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
SIZE = $(PREFIX)size

# 目录定义
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

# 源文件
C_SOURCES = $(wildcard $(SRC_DIR)/*.c)
ASM_SOURCES = $(wildcard $(SRC_DIR)/*.s)

# 目标文件
C_OBJECTS = $(C_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
ASM_OBJECTS = $(ASM_SOURCES:$(SRC_DIR)/%.s=$(BUILD_DIR)/%.o)
OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

# 链接脚本
LDSCRIPT = $(TARGET).ld

# 编译标志
CFLAGS = -mcpu=cortex-m4
CFLAGS += -mthumb
CFLAGS += -mfpu=fpv4-sp-d16
CFLAGS += -mfloat-abi=hard
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -fdata-sections
CFLAGS += -ffunction-sections
CFLAGS += -g
CFLAGS += -O2
CFLAGS += -I$(INC_DIR)

# 汇编标志
ASFLAGS = -mcpu=cortex-m4
ASFLAGS += -mthumb
ASFLAGS += -mfpu=fpv4-sp-d16
ASFLAGS += -mfloat-abi=hard
ASFLAGS += -g

# 链接标志
LDFLAGS = -mcpu=cortex-m4
LDFLAGS += -mthumb
LDFLAGS += -mfpu=fpv4-sp-d16
LDFLAGS += -mfloat-abi=hard
LDFLAGS += -specs=nano.specs
LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map,--cref
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -static
LDFLAGS += --specs=nosys.specs

# 默认目标
all: $(BUILD_DIR)/$(PROJECT).elf $(BUILD_DIR)/$(PROJECT).hex $(BUILD_DIR)/$(PROJECT).bin

# 创建ELF文件
$(BUILD_DIR)/$(PROJECT).elf: $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

# 创建HEX文件
$(BUILD_DIR)/$(PROJECT).hex: $(BUILD_DIR)/$(PROJECT).elf
	$(OBJCOPY) -O ihex $< $@

# 创建BIN文件
$(BUILD_DIR)/$(PROJECT).bin: $(BUILD_DIR)/$(PROJECT).elf
	$(OBJCOPY) -O binary -S $< $@

# 编译C文件
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

# 编译汇编文件
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# 创建构建目录
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 清理
clean:
	rm -rf $(BUILD_DIR)

# 烧录程序
flash: $(BUILD_DIR)/$(PROJECT).bin
	st-flash write $< 0x8000000

# 反汇编
disasm: $(BUILD_DIR)/$(PROJECT).elf
	$(OBJDUMP) -d $< > $(BUILD_DIR)/$(PROJECT).dis

# 显示大小信息
size: $(BUILD_DIR)/$(PROJECT).elf
	$(SIZE) -A -d $<

# 帮助信息
help:
	@echo "可用目标:"
	@echo "  all     - 编译项目 (默认)"
	@echo "  clean   - 清理构建文件"
	@echo "  flash   - 烧录程序到MCU"
	@echo "  disasm  - 生成反汇编文件"
	@echo "  size    - 显示程序大小信息"
	@echo "  help    - 显示此帮助信息"

.PHONY: all clean flash disasm size help