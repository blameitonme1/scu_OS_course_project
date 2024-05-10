#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include "scheduler.h"
#include <time.h>
#include <pwd.h> // 用于获取用户名
#include <signal.h>
#include <stdbool.h>
#define BUFLEN 1024  // 命令缓冲区长度
#define READY_QUEUE_SIZE 100  // 就绪队列大小

waitqueue *head = NULL; // 指向就绪队列的头结点

bool time_to_schedule_jobs = false;

// 初始化作业调度程序
void scheduler_init() {
    // 初始化就绪队列
    waitqueue *head = NULL;
    // 其他初始化代码...
}

// 定义信号处理函数
void signal_handler(int signum) {
    printf("Time slice expired, signal %d caught\n", signum);
}

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

// 提交新作业
int scheduler_enq(jobcmd *cmd) {
    // 创建新进程
    pid_t pid = fork();
    // 将作业信息添加到就绪队列
    jobinfo *new_job = malloc(sizeof(jobinfo));
    new_job->jid = 0; // 为作业分配唯一 ID
    new_job->pid = 0; // 为进程分配 ID
    strcpy(new_job->cmdarg, cmd->data);
    new_job->defpri = cmd->defpri;
    new_job->curpri = new_job->defpri;
    new_job->ownerid = cmd->owner;
    new_job->wait_time = 0;
    new_job->create_time = time(NULL);
    new_job->run_time = 0;
    new_job->state = READY;

    // 将作业添加到就绪队列
    waitqueue *new_node = malloc(sizeof(waitqueue));
    new_node->job = new_job;
    new_node->next = head;
    head = new_node;

    // 在标准输出上打印信息
    printf("Job enqueued successfully: jid=%d\n", new_job->jid);

    return new_job->jid;
}

// 查看作业状态
void scheduler_stat() {
    waitqueue *curr = head;
    
    while (curr != NULL) {
        jobinfo *job = curr->job;
        
        // 获取用户名
        struct passwd *user_info = getpwuid(job->ownerid);
        const char *username = user_info != NULL ? user_info->pw_name : "unknown";
        
        // 计算等待时间
        long wait_time_in_seconds = difftime(time(NULL), job->create_time);
        
        // 打印作业信息
        printf("PID: %d, Username: %s, Execution Time: %ld, Wait Time: %ld, Creation Time: %s, State: %s\n",
               job->pid, username,
               (long) job->run_time, wait_time_in_seconds,
               ctime(&job->create_time),
               job->state == READY ? "READY" : "RUNNING");
        
        curr = curr->next;
    }
}

// 移出作业
int scheduler_deq(int jid) {
}

// 主要调度循环
void scheduler_loop() {
}

void read_from_fifo(const char* fifo_name) {
    int fifo_fd;
    jobcmd cmd;

    fifo_fd = open(fifo_name, O_RDONLY);
    if (fifo_fd == -1) {
        perror("Failed to open FIFO");
        exit(EXIT_FAILURE);
    }

    while(1) {
        ssize_t bytes_read = read(fifo_fd, &cmd, sizeof(cmd));
        if (bytes_read > 0) {
            printf("Read from FIFO: %s\n", cmd.data);
        } else if (bytes_read == 0) {
            printf("End of FIFO reached.\n");
            break;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // 没有数据可读，稍后再试
            sleep(1);
            continue;
        } else {
            perror("Read error from FIFO");
            break;
        }
    }

    close(fifo_fd);
}

int main() {
    // scheduler_init();
    // // 注册信号处理函数,时间片耗尽通知进程

    // // 设置定时器参数，时间片10毫秒
    // struct itimerval timer;
    // timer.it_value.tv_sec = 0; // 初始时间，0秒后触发
    // timer.it_value.tv_usec = 10000; // 10毫秒
    // timer.it_interval.tv_sec = 0; // 重复间隔，之后每10毫秒触发一次
    // timer.it_interval.tv_usec = 10000; // 与首次触发相同

    // // 使用setitimer函数设置定时器
    // if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
    //     perror("setitimer failed");
    //     return 1;
    // }

    // 处理用户命令
    while (1) {
        // 从用户读取命令
        read_from_fifo(FIFO_NAME);
    }
    return 0;
}
