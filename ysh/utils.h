#ifndef UTILS_H
#define UTILS_H

#define max_command_length 100 // 最大命令长度
#define max_path_length 100 // 最大路径长度
char cwd[max_path_length]; // 当前工作目录
void print_prompt();
void goodbye(); 
int get_cwd();
int cd(char* command);
void execute_command(char* command);

#endif