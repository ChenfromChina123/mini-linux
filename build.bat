@echo off
REM Mini Linux Shell 编译脚本 (Windows)

echo 正在编译 Mini Linux Shell...

REM 编译所有 .c 文件为 .o 目标文件
gcc -Wall -g -c shell.c -o shell.o
gcc -Wall -g -c user.c -o user.o
gcc -Wall -g -c history.c -o history.o
gcc -Wall -g -c util.c -o util.o
gcc -Wall -g -c mycat.c -o mycat.o
gcc -Wall -g -c myrm.c -o myrm.o
gcc -Wall -g -c myvi.c -o myvi.o
gcc -Wall -g -c mytouch.c -o mytouch.o
gcc -Wall -g -c myecho.c -o myecho.o
gcc -Wall -g -c mycp.c -o mycp.o
gcc -Wall -g -c myls.c -o myls.o
gcc -Wall -g -c myps.c -o myps.o
gcc -Wall -g -c mycd.c -o mycd.o
gcc -Wall -g -c mymkdir.c -o mymkdir.o
gcc -Wall -g -c myagent.c -o myagent.o

REM 链接所有目标文件生成可执行文件
gcc -Wall -g -o mini_linux_shell.exe shell.o user.o history.o util.o mycat.o myrm.o myvi.o mytouch.o myecho.o mycp.o myls.o myps.o mycd.o mymkdir.o myagent.o

if %errorlevel% equ 0 (
    echo 编译成功！可执行文件：mini_linux_shell.exe
) else (
    echo 编译失败，请检查错误信息。
)

pause

