#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
void print_prompt(){
    /*
    * print out prompt using different colors.
    */
    printf("\033[0;31m"); // 设置颜色为红色
    printf("ysh");
    printf("\033[0;32m"); // $符号设置为绿色
    printf(" -> ");
    printf("\033[0;32m"); // $符号设置为绿色
    printf("$: ");
}
void goodbye(){
    /*
    * bye bye
    */
    printf("\033[0;32m"); // $符号设置为绿色
    printf("Thank you for using ");
    printf("\033[0;31m"); // 设置颜色为红色
    printf("ysh, ");
    printf("\033[0;32m"); // $符号设置为绿色
    printf("untill next time~\n");
}

void execute_command(char* command){
    /*
    * execute the command using system command.
    */
    command[strcspn(command, "\n")] = 0; // 去除换行符
    if(strcmp(command, "") == 0){
        // 输出为空
        printf("here!");
        return ;
    }
    int status = system(command);
    if (status == -1){
        printf("error occured when executing command %s", command);
        return ;
    }
}