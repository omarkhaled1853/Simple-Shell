#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

int max_words = 20, idx = 0;
char line[100] = {};
char *args[20] = {};
char variables[100][100] = {};
char values[100][100] = {};

// get input line from user
void parse_input(){
    fgets(line, sizeof(line), stdin);
}

// splite input line in the spaces
void evaluate_expression(){
    char tmp[100];
    int i = 0;
    while(line[i] != '\0'){
        tmp[i] = line[i];
        i++;
    }
    char *token = strtok(tmp, " \n");
    i = 0;
    while (token != NULL && i < max_words - 1) {
        args[i] = token;
        i++;
        token = strtok(NULL, " \n");
    }
    args[i] = NULL;   
}

// check if builtin shell 
bool shell_builtin(){
    return strcmp(args[0], "cd") == 0 || strcmp(args[0], "echo") == 0 || strcmp(args[0], "export") == 0;
}

// print current working directory
void cwd(){
    int id = fork();
    if(id == 0){
        char *buf = (char *)malloc(100*sizeof(char));
        getcwd(buf,100);
        printf("%s$ ", buf);
        exit(0);
    }
    else{
        wait(&id);
    }
}

// function on_child_exit()
//     reap_child_zombie()
//     write_to_log_file("Child terminated")


void cd(){
    if(args[1] == NULL || strcmp(args[1], "~") == 0){
        chdir("/home");
    }
    else if (strcmp(args[1], "..")){
        int i = strlen(args[1]) - 1;
        while(i >= 0){
            if(args[1][i] == '/'){
                args[1][i] = '\0';
                break;
            }
            i--;
        }
        chdir(args[1]);
    }
    else{
        chdir(args[1]);
    }
}

void echo(){
    int id = fork();
    if(id == 0){
        const char *start = strchr(line, '"');
        start++;
        const char *end = strchr(start, '"');
        for (const char *ptr = start; ptr < end; ptr++) {
            putchar(*ptr);
        }
        printf("\n");
        exit(0);
    }
    else{
        wait(&id);
    }
}

void export(){
    // int id = fork();
    // if(id == 0){
        const char *equal = strchr(args[1], '=');
        strncpy(variables[idx], args[1], equal - args[1]);
        variables[idx][equal - args[1]] = '\0';
        strcpy(values[idx], equal + 1);
        printf("%s\n", variables[idx]);
        printf("%s\n", values[idx]);
        idx++;
    //     exit(0);
    // }
    // else{
    //     wait(&id);
    // }

}

void execute_shell_bultin(){
    if (strcmp(args[0], "cd") == 0){
        cd();
    }
    else if (strcmp(args[0], "echo") == 0){
        echo();
    }
    else if (strcmp(args[0], "export") == 0){
        export();
    }
    
}
// execute non builtin shell 
void execute_command(){
    int id = fork();
    if (id == 0){
        execvp(args[0], args);
        printf("Error\n");
        exit(0);
    }
    else{
        wait(&id);
    }
}

void shell(){
    do{
        cwd();
        parse_input();
        evaluate_expression();    
        if(shell_builtin()){
            execute_shell_bultin();
        }
        else if (strcmp(args[0], "exit") == 0){
                exit(0);
        }
        else{
            execute_command();
        }
    }while (true);
}

void setup_environment(){
    chdir("/");
}

int main(){
    // register_child_signal(on_child_exit())
    setup_environment();
    shell();
    return 0;
}