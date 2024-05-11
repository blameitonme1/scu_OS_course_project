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
}

void sigchld_handler(int signum) {
    pid_t pid;
    int status;

    // 忽略已处理的SIGCHLD信号，防止信号被多次处理
    signal(SIGCHLD, SIG_IGN);

    // 使用waitpid获取子进程状态
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // 检查子进程状态
        if (WIFEXITED(status)) {
            // 子进程正常退出
            printf("Child %d exited with status %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSTOPPED(status)) {
            // 子进程被暂停
            printf("Child %d stopped by signal %d\n", pid, WSTOPSIG(status));
        } else if (WIFCONTINUED(status)) {
            // 子进程恢复运行
            printf("Child %d continued\n", pid);
        }
    }

    // 重新注册信号处理函数，以便处理后续的SIGCHLD信号
    signal(SIGCHLD, sigchld_handler);
}

void switch_job(pid_t current_pid, pid_t new_pid) {
    // 停止当前作业
    kill(current_pid, SIGSTOP);

    // 如果新的作业具有更高优先级，启动它
    if (new_pid != -1) {
        // 开始新的作业
        kill(new_pid, SIGCONT);
    }
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

// 生成唯一的作业ID
static int generate_unique_jid() {
    static int last_jid = 1;
    return last_jid++;
}

// 提交新作业
int scheduler_enq(jobcmd *cmd) {
    // 创建新进程
    pid_t pid = fork();
    if (pid < 0) {
        perror("Failed to fork");
        exit(EXIT_FAILURE);
    }

    // 父进程处理
    if (pid > 0) {
        // 将作业信息添加到就绪队列
        jobinfo *new_job = malloc(sizeof(jobinfo));

        new_job->jid = generate_unique_jid(); // 为作业分配唯一 ID
        new_job->pid = pid; // 为进程分配 ID
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
    } else {
        // 子进程执行作业
        _exit(EXIT_SUCCESS); // 子进程结束
    }
}

// 查看作业状态
void scheduler_stat() {
    waitqueue *curr = head;
    
    while (curr) {
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

// 查找并删除指定ID的作业
int scheduler_deq(int jid) {
    // 从就绪队列头部开始查找作业
    waitqueue *current_node = head;
    waitqueue *prev_node = NULL;
    while (current_node) {
        if (current_node->job->jid == jid) {
            // 找到作业，检查是否在运行
            if (current_node->job->state == RUNNING) {
                // 发送SIGTERM信号中止作业
                kill(current_node->job->pid, SIGTERM);
            }

            // 移除队列中的作业节点
            if (current_node->next) {
                prev_node->next = current_node->next;
            } else {
                // 如果是最后一个节点，更新头节点
                head = current_node->next;
            }

            // 释放作业信息
            free(current_node->job);
            free(current_node);

            return 1; // 成功删除
        }
        current_node = current_node->next;
    }

    return 0; // 未找到
}

// 调度函数
void schedule() {
    // 如果就绪队列为空，无需调度
    read_from_fifo(FIFO_NAME);
    if (head == NULL) {
        return;
    }
    int current_pid = -1;
    // 获取当前优先级最高的作业
    jobinfo *highest_pri_job = head->job;
    waitqueue *current_node = head->next;
    while (current_node) {
        if(current_node->job->state == DONE){
            continue; // 暂时忽略已经完成的作业
        }
        current_node->job->wait_time == 10;
        if(current_node->job->wait_time == 100){
            // 等待了100 毫秒，则将优先级增加
            current_node->job->curpri ++;
        }
        if (current_node->job->curpri > highest_pri_job->curpri) {
            highest_pri_job = current_node->job;
        }
        if(current_node->job->state == RUNNING){
            current_node->job->run_time += time_slice;
            current_node->job->state = READY; // 放入等待队列
            current_node->job->curpri = current_node->job->defpri; // 回复成默认的优先级
            current_pid = current_node->job->pid;
        }
        current_node = current_node->next;
    }

    // 更新作业状态为RUNNING
    highest_pri_job->state = RUNNING;

    // 进行作业切换
    switch_job(current_pid, highest_pri_job->pid);
}

int read_from_fifo(const char* fifo_name) {
    // 从FIFO中读一次
    int fifo_fd;
    jobcmd cmd;
    int has_command = 0;
    fifo_fd = open(fifo_name, O_RDONLY);
    if (fifo_fd == -1) {
        perror("Failed to open FIFO");
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_read = read(fifo_fd, &cmd, sizeof(cmd));
    if (bytes_read > 0) {
        printf("Read from FIFO: %s\n", cmd.data);
        has_command = 1;
    } else if (bytes_read == 0) {
        printf("End of FIFO reached.\n");
    } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // 没有数据可读，稍后再试
        sleep(1);
    } else {
        perror("Read error from FIFO");
    }
    // 有命令就处理命令
    if(has_command){
        switch(cmd.type){
            case ENQ:
                scheduler_enq(&cmd);
                break;
            case DEQ:
                scheduler_deq(cmd.defpri); // 储存了jid
                break;
            case STAT:
                scheduler_stat();
                break;
            default:
                printf("Invalid command\n");
                break;
        }
    }
    close(fifo_fd);
    return has_command;
}

// 注册SIGALRM信号处理器
void alarm_handler(int signum) {
    // 每次时间片到期就调度一次
    schedule();
}
int main() {
    scheduler_init();
    // 注册信号处理函数,时间片耗尽通知进程
    signal(SIGALRM, alarm_handler);
    // 设置定时器参数，时间片10毫秒
    struct itimerval timer;
    timer.it_value.tv_sec = 0; // 初始时间，0秒后触发
    timer.it_value.tv_usec = 10000; // 10毫秒
    timer.it_interval.tv_sec = 0; // 重复间隔，之后每10毫秒触发一次
    timer.it_interval.tv_usec = 10000; // 与首次触发相同

    // 使用setitimer函数设置定时器
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("setitimer failed");
        return 1;
    }
    while(1){
        // 等待用户自行终结
    }
    return 0;
}
