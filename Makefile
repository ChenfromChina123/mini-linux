CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11 -D_POSIX_C_SOURCE=200809L -Iinclude
LDFLAGS =

SRC_DIR = src
APP_DIR = $(SRC_DIR)/app
CORE_DIR = $(SRC_DIR)/core
CMD_DIR = $(SRC_DIR)/commands
BIN_DIR = bin
OBJ_DIR = obj
SCRIPT_DIR = scripts

TARGET = bin/mini_shell

CORE_SOURCES = \
	$(CORE_DIR)/shell.c \
	$(CORE_DIR)/util.c \
	$(CORE_DIR)/input.c \
	$(wildcard $(CORE_DIR)/history/*.c) \
	$(wildcard $(CORE_DIR)/user/*.c)

INTEGRATED_COMMANDS = myagent mycat mycd mycp myecho myls mymkdir myps myrm mytouch myvi
INTEGRATED_CMD_SOURCES = $(addprefix $(CMD_DIR)/,$(addsuffix .c,$(INTEGRATED_COMMANDS)))

APP_SOURCES = $(wildcard $(APP_DIR)/*.c)
SOURCES = $(APP_SOURCES) $(CORE_SOURCES) $(INTEGRATED_CMD_SOURCES)

# 将源文件映射到对象文件
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))

STANDALONE_COMMANDS = $(INTEGRATED_COMMANDS) mychmod mykill myhistory
STANDALONE_TARGETS = $(addprefix $(BIN_DIR)/,$(STANDALONE_COMMANDS))
STANDALONE_COMMON_SOURCES = $(CORE_DIR)/util.c $(CORE_DIR)/input.c

SHELL_SCRIPTS = $(wildcard $(SCRIPT_DIR)/*.sh)
USER_SCRIPTS = myuseradd.sh myuserdel.sh mypasswd.sh

.PHONY: all
all: directories $(TARGET) standalone scripts
	@echo "========================================="
	@echo "编译完成！"
	@echo "========================================="
	@echo "主程序: ./$(TARGET)"
	@echo "独立命令: $(BIN_DIR)/{mycat,mytouch,mycp,...}"
	@echo "Shell脚本: $(BIN_DIR)/*.sh"
	@echo "========================================="

$(TARGET): $(OBJECTS)
	@echo "链接主程序: $@"
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | directories
	@echo "编译: $< -> $@"
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: standalone
standalone: directories $(STANDALONE_TARGETS)
	@echo "所有独立命令编译完成"

$(BIN_DIR)/%: $(CMD_DIR)/%.c $(STANDALONE_COMMON_SOURCES) | $(BIN_DIR)
	@echo "编译命令: $@"
	$(CC) $(CFLAGS) -DMINI_LINUX_STANDALONE -o $@ $^ $(LDFLAGS)

.PHONY: directories
directories:
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

.PHONY: scripts
scripts: $(SHELL_SCRIPTS) | $(BIN_DIR)
	@echo "复制Shell脚本到 $(BIN_DIR)/"
	@for script in $(SHELL_SCRIPTS); do \
		base=$$(basename $$script); \
		cp $$script $(BIN_DIR)/$$base; \
		chmod +x $(BIN_DIR)/$$base; \
		echo "  - $$base"; \
	done
	@# 确保用户管理脚本可以通过去掉 .sh 的方式调用
	@cd $(BIN_DIR) && for s in $(USER_SCRIPTS); do \
		base=$${s%.sh}; \
		cp $$s $$base 2>/dev/null || true; \
		chmod +x $$base 2>/dev/null || true; \
	done

.PHONY: agent
agent:
	@echo "========================================="
	@echo "构建小晨Agent可执行文件..."
	@echo "========================================="
	@if [ -x "$(SCRIPT_DIR)/build_agent.sh" ]; then \
		$(SCRIPT_DIR)/build_agent.sh; \
	else \
		echo "错误: $(SCRIPT_DIR)/build_agent.sh 不存在或不可执行"; \
		exit 1; \
	fi

.PHONY: all-with-agent
all-with-agent: all agent
	@echo "========================================="
	@echo "完整构建完成（包括Agent）！"
	@echo "========================================="

.PHONY: clean
clean:
	@echo "清理编译产物..."
	@if exist $(BIN_DIR) rd /s /q $(BIN_DIR)
	@if exist $(OBJ_DIR) rd /s /q $(OBJ_DIR)
	@echo "清理完成。"

.PHONY: rebuild
rebuild: clean all

.PHONY: run
run: all
	@echo "启动Mini Linux Shell..."
	@./$(TARGET)

.PHONY: test-compile
test-compile:
	@echo "测试编译所有源文件..."
	@for src in $(SOURCES) $(addprefix $(CMD_DIR)/,$(addsuffix .c,$(STANDALONE_COMMANDS))); do \
		echo "测试: $$src"; \
		$(CC) $(CFLAGS) -c $$src -o /dev/null; \
	done
	@echo "测试编译完成。"

.PHONY: help
help:
	@echo "========================================="
	@echo "Mini Linux Shell - Makefile帮助"
	@echo "========================================="
	@echo "可用目标："
	@echo "  make               - 编译主程序/独立命令/脚本（默认）"
	@echo "  make clean         - 清理编译产物"
	@echo "  make rebuild       - 重新编译"
	@echo "  make run           - 编译并运行主程序"
	@echo "  make standalone    - 仅编译独立命令到 bin/"
	@echo "  make scripts       - 仅复制脚本到 bin/"
	@echo "  make test-compile  - 仅测试编译（不链接）"
	@echo "  make agent         - 构建小晨Agent可执行文件"
	@echo "========================================="
	@echo "  make uninstall - 从系统卸载"
	@echo "  make test-compile - 测试编译"
	@echo "  make help     - 显示此帮助信息"
	@echo "========================================="

# 依赖关系（可选，用于增量编译）
$(SHELL_BIN): $(SHELL_MAIN)
$(CMD_BINS): $(CMD_DIR)/%.c

