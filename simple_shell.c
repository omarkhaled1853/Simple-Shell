#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

int max_words = 20;
char line[100] = {};
char *args[20] = {};
char *current_directory = "/";

// get input line from user
void parse_input(){
    fgets(line, sizeof(line), stdin);
}

// splite input line in the spaces
void evaluate_expression(){
    char *token = strtok(line, " \n");
    int i = 0;
    while (token != NULL && i < max_words - 1) {
        args[i] = token;
        i++;
        token = strtok(NULL, " \n");
    }
    args[i] = NULL;   
}


// function on_child_exit()
//     reap_child_zombie()
//     write_to_log_file("Child terminated")



// 
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
bool shell_builtin(){
    return strcmp(args[0], "cd") == 0 || strcmp(args[0], "echo") == 0 || strcmp(args[0], "export") == 0;
}
void setup_environment(){
    chdir(current_directory);
    args[0] = "pwd";
    args[1] = NULL;
    execute_command();
}
void cd(){
        // if(strcmp(args[1], "~") == 0 || strcmp(args[1], NULL) == 0){
        //     chdir("/home");
        // }
        // else if (strcmp(args[1], "..")){
            // int i = strlen(args[1]) - 1;
            // while(i >= 0){
            //     if(args[1][i] == '/'){
            //         break;
            //     }
            // }
        // }
        // printf("%s\n", args[1]);
        chdir(args[1]);
        current_directory = args[1];
}

void echo(){
    int id = fork();
    if(id == 0){
        char *token = strtok(line, " \n");
        token = strtok(NULL, " \n");
        printf("%s\n", line);
        exit(0);
    }
    else{
        wait(&id);
    }
}

// void export(){

// }

void execute_shell_bultin(){
    if (strcmp(args[0], "cd") == 0){
        cd();
    }
    else if (strcmp(args[0], "echo") == 0){
        echo();
    }
    else if (strcmp(args[0], "export") == 0){
        // export();
    }
    
}
void shell(){
    do{
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
        setup_environment();
    }while (true);
}


int main(){
    // register_child_signal(on_child_exit())
    setup_environment();
    shell();
    return 0;
}