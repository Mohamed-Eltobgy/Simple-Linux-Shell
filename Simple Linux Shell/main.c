#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#define MAX 250

// this function is used to reap child processes and register them in a text file
void on_child(){
    int status, pid;
    while((pid = waitpid(-1,&status,WNOHANG)) > 0){

    }
    char *logFile = "terminated_processes.txt";
    FILE *fp = fopen(logFile, "a+");
    if(fp == NULL){
        return -1;
    }
    fprintf(fp,"Child terminated1\n");
    fclose(fp);
}
//this function is used to start from a specifc directory.
void setup_environment(){
    chdir("/home/user");
}
//this function takes input and analyze it to be used later
void shell(){
    int flag;
    do{
        char line[MAX];
        fgets(line, MAX,stdin);
        char cmds[20][MAX];
        int i, j=0, temp = 0;
        for(i=0; i<20;i++){
            int boo = 1;
            int counter = 0;
            for(j= temp; j < strlen(line);j++){
                if(line[j] == ' ' ){
                    break;

                }else if(line[j] == '\0' || line[j] == '\n'){
                    boo = 0;
                }else{
                    cmds[i][counter] = line[j];
                    counter++;
                    cmds[i][counter] = '\0';
                }
            }
            temp = j+1;
            if(boo == 0){
                break;
            }
        }
        flag = strcmp(cmds[0], "exit");
        if(strcmp(cmds[0],"cd") == 0 || strcmp(cmds[0],"echo") == 0 || strcmp(cmds[0],"export") == 0){
            execute_shell_builtin(cmds, line);
        }else if(flag == 0){
            exit(0);
        }else{
            execute_command(cmds, i+1);
        }
    }while(1); // infinite loop that can be broken only be exit command.
}
//implementing functions that cant be used directly.
void execute_shell_builtin(char cmds[20][MAX], char line[MAX]){
    if(strcmp(cmds[0], "cd")== 0){
        if(strcmp(cmds[1],"~") == 0){
            chdir("/home");     //chdir is just an alias to cd.
        }else if(strcmp(cmds[1],"-") == 0){
            chdir("..");
            pid_t c;
            c = fork();
            if(c == 0){
                char* command = "pwd";
                char* parameters[]= {"pwd", NULL, NULL};
                execvp(command, parameters);
            }else{
                on_child();
            }
        }else{
            chdir(cmds[1]);
        }
    }else if(strcmp(cmds[0], "export") == 0){
        int before=0, after=0, flag = 0;
        for(int i= 7; i<strlen(line);i++){
            if(line[i] == '='){
                flag = 1;
            }
            if(flag == 0){
                before++;
            }else{
                if(line[i] != '"'){
                    after++;
                }
            }
        }
        char name[before];
        char exp[after];
        int i, temp;
        for(i= 7; i<strlen(line); i++){
            if(line[i] == '='){
                name[i] = '\0';
                i++;
                break;
            }else{
                name[i-7] = line[i];
            }
        }
        temp = i;
        for(i= temp; i<strlen(line); i++){
            if(line[i] == '"'){
                temp++;
                continue;
            }else{
                exp[i-temp] = line[i];
            }
        }
        exp[after] = '\0';
        setenv(name,exp,1);
    }else{
        char str[MAX];
        int h =0;
        for(int i= 6; i<strlen(line) -2; i++){
            if(line[i] == '$'){
                char str2[MAX];
                int counter = 0;
                for(int j=i+1; j<strlen(line);j++){
                    if(line[j] == ' ' || line[j] == '"'){
                        break;
                    }else{
                        str2[j-i-1] = line[j];
                        counter++;
                    }
                }
                str2[counter] = '\0';
                char* str3 = getenv(str2);
                for(int k = 0; k<strlen(str3)-1;k++){
                    str[h] = str3[k];
                    h++;
                }
                i+=2;
            }else if(line[i] =='"'){
                break;
            }else{
                str[h] = line[i];
                h++;
            }
        }
        str[h] = '\0';
        printf("%s\n",str);
    }
}

void execute_command(char cmds[20][MAX], int words){
    pid_t child_id;
    int status;
    child_id = fork();  //using fork to avoid termination of whole program.
    if(cmds[0] == "firefox"){
        if(cmds[1] == '&'){
            if(child_id == 0){
                char* command = cmds[0];
                char* parameters[words+1];
                for(int i=0; i< words; i++){
                    parameters[i] = cmds[i];
                }
                parameters[words] = NULL;
                execvp(command, parameters);
            }else{
                on_child();
            }
        }else{
            if(child_id == 0){
                char* command = cmds[0];
                char* parameters[words+1];
                for(int i=0; i< words; i++){
                    parameters[i] = cmds[i];
                }
                parameters[words] = NULL;
                execvp(command, parameters);
            }else{
                waitpid(child_id, &status,0);
            }
        }
    }else{
        if(child_id < 0){
            perror("Unsuccessful fork");
            exit(EXIT_FAILURE);
        }else if(child_id == 0){
            char* command = cmds[0];
            char* parameters[words+1];
            for(int i=0; i< words; i++){
                parameters[i] = cmds[i];
            }
            parameters[words] = NULL;
            if (execvp(command, parameters) < 0){
                perror("Unsuccessful instruction");
            }
        }else{
            on_child();
        }
    }
}

int main()
{
    signal (SIGCHLD, on_child);
    setup_environment();
    shell();
}
