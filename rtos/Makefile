# STM32 BluePill Simple RTOS Makefile

# 项目名称
PROJECT = simple_rtos

# 目标MCU
MCU = cortex-m3
TARGET = stm32f103c8t6

# 工具链前缀
PREFIX = arm-none-eabi-
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
OBJ_DIR = $(BUILD_DIR)/obj

# 源文件
C_SOURCES = $(wildcard $(SRC_DIR)/*.c)
ASM_SOURCES = $(wildcard $(SRC_DIR)/*.s)

# 目标文件
C_OBJECTS = $(C_SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
ASM_OBJECTS = $(ASM_SOURCES:$(SRC_DIR)/%.s=$(OBJ_DIR)/%.o)
OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

# 编译标志
CFLAGS = -mcpu=$(MCU) -mthumb -mfloat-abi=soft
CFLAGS += -DSTM32F103xB -DUSE_HAL_DRIVER
CFLAGS += -I$(INC_DIR)
CFLAGS += -Wall -Wextra -Wno-unused-parameter
CFLAGS += -fdata-sections -ffunction-sections
CFLAGS += -g -gdwarf-2

# 优化级别
OPT = -Os
CFLAGS += $(OPT)

# 汇编标志
ASFLAGS = -mcpu=$(MCU) -mthumb -mfloat-abi=soft
ASFLAGS += -g -gdwarf-2

# 链接标志
LDFLAGS = -mcpu=$(MCU) -mthumb -mfloat-abi=soft
LDFLAGS += -specs=nano.specs -specs=nosys.specs
LDFLAGS += -T$(PROJECT).ld
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map,--cref
LDFLAGS += -Wl,--print-memory-usage

# 默认目标
all: $(BUILD_DIR)/$(PROJECT).elf $(BUILD_DIR)/$(PROJECT).hex $(BUILD_DIR)/$(PROJECT).bin

# 创建目录
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

# 编译C文件
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# 编译汇编文件
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.s | $(OBJ_DIR)
	@echo "Assembling $<"
	@$(AS) $(ASFLAGS) -c $< -o $@

# 链接生成ELF文件
$(BUILD_DIR)/$(PROJECT).elf: $(OBJECTS) | $(BUILD_DIR)
	@echo "Linking $@"
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@$(SIZE) $@

# 生成HEX文件
$(BUILD_DIR)/$(PROJECT).hex: $(BUILD_DIR)/$(PROJECT).elf
	@echo "Creating $@"
	@$(OBJCOPY) -O ihex $< $@

# 生成BIN文件
$(BUILD_DIR)/$(PROJECT).bin: $(BUILD_DIR)/$(PROJECT).elf
	@echo "Creating $@"
	@$(OBJCOPY) -O binary -S $< $@

# 创建构建目录
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# 反汇编
disasm: $(BUILD_DIR)/$(PROJECT).elf
	@$(OBJDUMP) -d $< > $(BUILD_DIR)/$(PROJECT).dis

# 清理
clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD_DIR)

# 烧录 (使用st-flash)
flash: $(BUILD_DIR)/$(PROJECT).bin
	@echo "Flashing $(PROJECT).bin to STM32..."
	st-flash write $< 0x8000000

# 烧录 (使用openocd)
flash-openocd: $(BUILD_DIR)/$(PROJECT).elf
	@echo "Flashing with OpenOCD..."
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program $< verify reset exit"

# 调试 (使用gdb)
debug: $(BUILD_DIR)/$(PROJECT).elf
	@echo "Starting GDB debug session..."
	$(PREFIX)gdb $<

# 显示大小信息
size: $(BUILD_DIR)/$(PROJECT).elf
	@$(SIZE) --format=berkeley $<

# 显示帮助
help:
	@echo "Available targets:"
	@echo "  all          - Build the project"
	@echo "  clean        - Clean build files"
	@echo "  flash        - Flash using st-flash"
	@echo "  flash-openocd- Flash using OpenOCD"
	@echo "  debug        - Start GDB debug session"
	@echo "  disasm       - Generate disassembly"
	@echo "  size         - Show size information"
	@echo "  help         - Show this help"

# 伪目标
.PHONY: all clean flash flash-openocd debug disasm size help

# 依赖关系
-include $(OBJECTS:.o=.d)