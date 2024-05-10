#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>

#define BUFLEN 1024  // 命令缓冲区长度
#define READY_QUEUE_SIZE 100  // 就绪队列大小

#define FIFO_NAME "/tmp/myfifo"

enum jobstate {
    // 所有的状态
    READY,  RUNNING,  DONE
};

// 作业信息结构
typedef struct jobinfo {
    int jid;                // 作业 ID
    int pid;                // 进程 ID
    char cmdarg[BUFLEN]; // 命令参数
    int defpri;          // 默认优先级 default primacy
    int curpri;          // 当前优先级 current primacy
    int ownerid;         // 作业所有者 ID
    int wait_time;       // 等待时间
    time_t create_time; // 作业创建时间
    int run_time;       // 作业运行时间
    enum jobstate state; // 作业状态
} jobinfo;

// 作业命令结构
typedef struct jobcmd {
    enum cmdtype { ENQ, DEQ, STAT } type; // command类型
    int argnum; // 参数数量
    int owner; // 所有者
    int defpri; // 一开始给的默认优先级
    char data[BUFLEN]; // 具体命令？
} jobcmd;

// 就绪队列结构
typedef struct waitqueue {
    struct waitqueue *next;
    jobinfo *job;
} waitqueue;

// 初始化作业调度程序
void scheduler_init();

// 创建FIFO
void create_FIFO(const char *filename);

// 提交新作业
int scheduler_enq(jobcmd *cmd);

// 查看作业状态
void scheduler_stat();

// 移出作业
int scheduler_deq(int jid);

// 主要调度循环
void scheduler_loop();

void signal_handler(int signum);
#endif // SCHEDULER_H