#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "./utils.h"
#include "./utils.c"

int main(){
    // 初始化cwd
    if(get_cwd() != 0){
        return 1;
    }
    printf("type %s or %s to exit ysh!\n", "\"exit\"","\"Exit\"");
    char command[max_command_length];

    while(1){
        print_prompt();
        fgets(command, sizeof(command), stdin);
        // printf("command is %s", command);
        if((strcmp(command, "exit\n") == 0 || strcmp(command, "Exit\n") == 0)){
            break;
        }
        execute_command(command);
    }
    goodbye();
    return 0;
}