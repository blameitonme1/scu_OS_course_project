#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "scheduler.h"

void create_FIFO(const char *filename) {
    mode_t mode = 0644; // 权限模式

    if (mkfifo(filename, mode) == -1) {
        if (errno == EEXIST) {
            printf("FIFO already exists: %s\n", filename);
        } else {
            perror("failed to create FIFO file");
            exit(1);
        }
    }
}
char **split_command(const char *command, int *argc) {
    *argc = 0;
    if(strlen(command) == 0){
        return NULL;
    }
    char *copy = strdup(command); // 创建命令的副本，strtok会修改原始字符串
    char **argv = malloc(2 * sizeof(char*)); // 初始分配两个元素的空间，包括NULL终止符

    char *token = strtok(copy, " ");
    while (token != NULL) {
        // 为每个参数分配内存并复制字符串
        char *arg_copy = strdup(token);
        if (!arg_copy) {
            perror("Failed to allocate memory for an argument");
            exit(EXIT_FAILURE);
        }
        argv[*argc] = arg_copy;
        (*argc)++;
        // printf("%s\n", argv[*argc-1]);

        // 检查是否需要扩展argv
        if (*argc >= 2 && (*argc + 1) * sizeof(char*) > sizeof(argv)) {
            argv = realloc(argv, (*argc + 1) * sizeof(char*));
            if (!argv) {
                perror("Failed to reallocate memory for arguments");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, " ");
    }
    free(copy); // 释放复制的字符串
    argv[*argc] = NULL; // 添加NULL终止符
    return argv;
}

// 从标准输入读取命令并写入FIFO
void write_to_fifo() {
    create_FIFO(FIFO_NAME);
    char input[1024];
    jobcmd cmd;
    int fifo_fd;
    int argc = 0;
    while (1) {
        printf("Enter command: \n");
        fgets(input, sizeof(input), stdin);

        // 剪掉末尾的换行符（如果有的话）
        if (input[strlen(input) - 1] == '\n') {
            input[strlen(input) - 1] = '\0';
        }
        // 解析命令
        char **args = split_command(input, &argc);
        if (!args) {
            printf("Error parsing command.\n");
            continue;
        }
        // 初始化jobcmd结构体
        cmd.argnum = 1;
        cmd.owner = getuid(); // 使用getuid()获取当前用户ID作为所有者
        cmd.defpri = 0; // 根据需要设置默认优先级
        // 解析具体什么命令
        if (strcmp(args[0], "enq") == 0) {
            // 作业入队
            cmd.type = ENQ;
            int priority = 0; // 默认优先级
            if (args[1] && strcmp(args[1], "-p") == 0) {
                priority = atoi(args[2]);
                if (priority > 3 || priority < 0) {
                    priority = 0;
                }
                cmd.defpri = priority;
                cmd.argnum += 2; // 跳过"-p"和其后的参数计数
            }
            if (strlen(args[3]) > 0 && args[3][0] == '/') {
                strcpy(cmd.data, args[cmd.argnum++]); // 复制可执行文件路径
            } else {
                printf("Error: e_file must be an absolute path.\n");
                continue;
            }
            // 复制剩余的参数到data
            while (cmd.argnum < strlen(input) && args[cmd.argnum]) {
                strcat(cmd.data, " "); // 添加空格分隔参数
                strcat(cmd.data, args[cmd.argnum++]);
            }
        } else if (strcmp(args[0], "deq") == 0) {
            cmd.type = DEQ;
            if (strlen(args[1]) > 0) {
                cmd.defpri = atoi(args[1]); // 假设jid作为优先级处理，根据实际情况调整
            } else {
                printf("Error: Missing jid.\n");
                continue;
            }
        } else if (strcmp(args[0], "stat") == 0) {
            cmd.type = STAT;
        } else {
            printf("Unknown command.\n");
            continue;
        }

        // 打开FIFO进行写操作
        printf("here!\n");
        fifo_fd = open(FIFO_NAME, O_WRONLY);
        if (fifo_fd == -1) {
            perror("Failed to open FIFO for writing");
            exit(EXIT_FAILURE);
        }
        printf("here!\n");
        // 写入jobcmd结构体到FIFO
        // 增加时间调试信息
        ssize_t bytes_written = write(fifo_fd, &cmd, sizeof(cmd));
        if (bytes_written != sizeof(cmd)) {
            perror("Failed to write to FIFO");
            close(fifo_fd);
            exit(EXIT_FAILURE);
        }
        printf("here!\n");
        // 关闭FIFO
        close(fifo_fd);
    }
}

int main() {
    write_to_fifo();
    return 0;
}