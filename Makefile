# Mini Linux Shell Makefile

# 编译器
CC = gcc

# 编译选项
CFLAGS = -Wall -g

# 目标文件
TARGET = mini_linux_shell

# C语言程序源文件
C_SRCS = mycat.c myrm.c myvi.c mytouch.c myecho.c mycp.c myls.c myps.c mycd.c mymkdir.c

# 目标文件列表
C_OBJS = $(C_SRCS:.c=.o)

# 默认目标
all: $(TARGET)

# 生成shell主程序，包含所有命令实现
$(TARGET): shell.o user.o history.o util.o $(C_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# 编译源文件为目标文件
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理生成的文件
ifeq ($(OS),Windows_NT)
RM = del
EXE = $(TARGET).exe
else
RM = rm -f
EXE = $(TARGET)
endif

clean:
	$(RM) $(C_OBJS) shell.o user.o history.o util.o $(EXE)

# 伪目标
.PHONY: all clean
