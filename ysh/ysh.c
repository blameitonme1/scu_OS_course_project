#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "./ysh.h"
#include "./utils.c"

int command_size_limit = 400; // 命令的最大字数
int main(){
    printf("type %s or %s to exit ysh!\n", "\"exit\"","\"Exit\"");
    print_prompt();
    char command[command_size_limit];
    fgets(command, sizeof(command), stdin);
    printf("%s", command);
    while(1){
        print_prompt();
        fgets(command, sizeof(command), stdin);
        printf("command is %s", command);
        if((strcmp(command, "exit\n") == 0 || strcmp(command, "Exit\n") == 0)){
            break;
        }
        execute_command(command);
    }
    goodbye();
    return 0;
}