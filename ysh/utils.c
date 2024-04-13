#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "./utils.h"

/*不同颜色打出ysh的prompt*/
void print_prompt(){
    printf("\033[0;31m"); // 设置颜色为红色
    printf("ysh");
    printf("\033[0;32m"); // $符号设置为绿色
    printf(" -> ");
    printf("\033[0;32m"); // $符号设置为绿色
    printf("$: ");
}

/*bye bye*/
void goodbye(){
    printf("\033[0;32m"); // $符号设置为绿色
    printf("Thank you for using ");
    printf("\033[0;31m"); // 设置颜色为红色
    printf("ysh, ");
    printf("\033[0;32m"); // $符号设置为绿色
    printf("untill next time~\n");
}

/*获取当前工作目录*/
int get_cwd(){
    if(getcwd(cwd, sizeof(cwd)) == NULL){
        perror("cant get cwd");
        return 1; // 错误退出
    }
    return 0;
}

/*处理cd命令*/
int cd(char* command){
    char* arg = command + 3; // 读取cd的参数
    if(chdir(arg) != 0){
        // 未能成功切换
        perror("cant change cwd");
        return 1;
    }
    else if(get_cwd() != 0){
        return 1;
    }
    return 0;
}

/*执行命令*/
void execute_command(char* command){

    command[strcspn(command, "\n")] = 0; // 去除换行符
    if(strcmp(command, "") == 0){
        // 输出为空
        printf("no command at all!\n");
        return;
    }
    if(strncmp(command, "cd", 2) == 0){
        // 检查是否cd开头
        if(cd(command) != 0){
            // 命令失败，退出
            exit(EXIT_FAILURE);
        }
        else{
            return ;
        }
    }
    int status = system(command);
    if (status == -1){
        printf("error occured when executing command %s", command);
        return ;
    }
}