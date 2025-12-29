# Mini Linux Shell Makefile

# 编译器
CC = gcc

# 目录定义
INC_DIR = include
SRC_CORE_DIR = src/core
SRC_CMD_DIR = src/commands
OBJ_DIR = obj

# 编译选项 - 添加 -Iinclude 搜索路径
# 设置控制台编码为 UTF-8
CFLAGS = -Wall -g -I$(INC_DIR) -finput-charset=UTF-8 -fexec-charset=UTF-8

# 目标文件
TARGET = mini_linux_shell

# 核心源文件
CORE_SRCS = $(wildcard $(SRC_CORE_DIR)/*.c)
# 命令源文件
CMD_SRCS = $(wildcard $(SRC_CMD_DIR)/*.c)

# 合并所有源文件
ALL_SRCS = $(CORE_SRCS) $(CMD_SRCS)

# 目标文件列表
CORE_OBJS = $(patsubst $(SRC_CORE_DIR)/%.c, $(OBJ_DIR)/%.o, $(CORE_SRCS))
CMD_OBJS = $(patsubst $(SRC_CMD_DIR)/%.c, $(OBJ_DIR)/%.o, $(CMD_SRCS))
OBJS = $(CORE_OBJS) $(CMD_OBJS)

# 默认目标
all: $(OBJ_DIR) $(TARGET)

# 创建 obj 目录
$(OBJ_DIR):
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)

# 生成shell主程序
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# 编译核心源文件
$(OBJ_DIR)/%.o: $(SRC_CORE_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# 编译命令源文件
$(OBJ_DIR)/%.o: $(SRC_CMD_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理生成的文件
ifeq ($(OS),Windows_NT)
RM = rmdir /s /q
DEL = del /q
EXE = $(TARGET).exe
else
RM = rm -rf
DEL = rm -f
EXE = $(TARGET)
endif

clean:
	@if exist $(OBJ_DIR) $(RM) $(OBJ_DIR)
	@if exist $(EXE) $(DEL) $(EXE)
	@if exist $(TARGET) $(DEL) $(TARGET)

# 伪目标
.PHONY: all clean
