#ifndef COMMAND_H
#define COMMAND_H

// 命令函数声明
int cmd_mycat(int argc, char *argv[]);
int cmd_myrm(int argc, char *argv[]);
int cmd_myvi(int argc, char *argv[]);
int cmd_mytouch(int argc, char *argv[]);
int cmd_myecho(int argc, char *argv[]);
int cmd_mycp(int argc, char *argv[]);
int cmd_myls(int argc, char *argv[]);
int cmd_myps(int argc, char *argv[]);
int cmd_exit(int argc, char *argv[]);
int cmd_clear(int argc, char *argv[]);
int cmd_help(int argc, char *argv[]);
int cmd_history(int argc, char *argv[]);
int cmd_useradd(int argc, char *argv[]);
int cmd_userdel(int argc, char *argv[]);
int cmd_mycd(int argc, char *argv[]);
int cmd_mymkdir(int argc, char *argv[]);
int cmd_users(int argc, char *argv[]);
int cmd_passwd(int argc, char *argv[]);
int cmd_agent(int argc, char *argv[]);

#endif // COMMAND_H
