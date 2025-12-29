@echo off
REM Mini Linux Shell 编译脚本 (Windows - MinGW)
REM 注意：由于项目使用了大量的 POSIX 系统调用，此脚本在原生 Windows 下可能编译失败。
REM 建议在 WSL 或 Cygwin 环境下使用 Makefile 编译。

echo 正在编译 Mini Linux Shell (分层结构版)...

REM 创建 obj 目录
if not exist obj mkdir obj

REM 编译选项
set CFLAGS=-Wall -g -Iinclude -finput-charset=UTF-8 -fexec-charset=UTF-8

echo [1/3] 编译核心组件...
gcc %CFLAGS% -c src/core/shell.c -o obj/shell.o
gcc %CFLAGS% -c src/core/user.c -o obj/user.o
gcc %CFLAGS% -c src/core/history.c -o obj/history.o
gcc %CFLAGS% -c src/core/util.c -o obj/util.o

echo [2/3] 编译命令实现...
gcc %CFLAGS% -c src/commands/mycat.c -o obj/mycat.o
gcc %CFLAGS% -c src/commands/myrm.c -o obj/myrm.o
gcc %CFLAGS% -c src/commands/myvi.c -o obj/myvi.o
gcc %CFLAGS% -c src/commands/mytouch.c -o obj/mytouch.o
gcc %CFLAGS% -c src/commands/myecho.c -o obj/myecho.o
gcc %CFLAGS% -c src/commands/mycp.c -o obj/mycp.o
gcc %CFLAGS% -c src/commands/myls.c -o obj/myls.o
gcc %CFLAGS% -c src/commands/myps.c -o obj/myps.o
gcc %CFLAGS% -c src/commands/mycd.c -o obj/mycd.o
gcc %CFLAGS% -c src/commands/mymkdir.c -o obj/mymkdir.o
gcc %CFLAGS% -c src/commands/myagent.c -o obj/myagent.o

echo [3/3] 链接生成可执行文件...
gcc %CFLAGS% -o mini_linux_shell.exe obj/*.o

if %errorlevel% equ 0 (
    echo 编译成功！可执行文件：mini_linux_shell.exe
) else (
    echo 编译失败，请检查错误信息。
    echo 提示：Windows 下编译需要 POSIX 兼容环境（如 MSYS2 或 Cygwin）。
)

pause
