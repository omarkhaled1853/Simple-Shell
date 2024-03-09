#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

int max_words = 20, idx = 0;
char line[100];
char **args = NULL;
char variables[100][100];
char values[100][100];
char *filepath = "logfile.txt";

// get input line from user
void parse_input(){
    fgets(line, sizeof(line), stdin);
    int length = strlen(line);
    if (line[length - 1] == '\n') {
        line[length - 1] = '\0';
    }
}

// replace variables after $ sign
void replace_variable() {
char *dollar_sign = strchr(line, '$');
    if (dollar_sign != NULL) {
        // Find the position of the first space after '$'
        char *space_after_dollar = dollar_sign + 1;
        while (*space_after_dollar != ' ' && *space_after_dollar != '\0') {
            space_after_dollar++;
        }
        

        // Calculate the length of the variable name (after '$' until the first space)
        int variable_length = space_after_dollar - dollar_sign - 1;

        // Copy the variable name (after '$' until the first space)
        char variable_name[100];
        strncpy(variable_name, dollar_sign + 1, variable_length);
        variable_name[variable_length] = '\0';

        // Search for the variable in variables array
        for (int i = 0; i < 100 && variables[i][0] != '\0'; i++) {
            if (strcmp(variables[i], variable_name) == 0) {
                // Calculate the length of the variable value
                int value_length = strlen(values[i]);
                
                // Calculate the length of the remaining part after the variable name
                int remaining_length = strlen(dollar_sign + variable_length + 1);

                // If the value is longer than the variable name, we need to shift the remaining part
                if (value_length > variable_length) {
                    // Shift the remaining part to make space for the value
                    memmove(dollar_sign + value_length, dollar_sign + variable_length + 1, remaining_length + 1);
                }
                strncpy(dollar_sign, values[i], value_length);
                break;
            }
        }
    }
}

// splite input line in the spaces
void evaluate_expression(){
    replace_variable();
    args = malloc(max_words * sizeof(char*));
    if (args == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);
    }

    char tmp[100];
    int i = 0;
    while(line[i] != '\0'){
        tmp[i] = line[i];
        i++;
    }
    tmp[i] = '\0';
    char *token = strtok(tmp, " \n");
    i = 0;
    while (token != NULL && i < max_words - 1) {
        args[i] = strdup(token);
        if (args[i] == NULL) {
            printf("Memory allocation failed!\n");
            exit(1);
        }
        i++;
        token = strtok(NULL, " \n");
    }
    args[i] = NULL;
}

// check if builtin shell 
bool shell_builtin(){
    return strcmp(args[0], "cd") == 0 || strcmp(args[0], "echo") == 0 || strcmp(args[0], "export") == 0;
}

// check for zombie process
bool check_background_process(){
    for(int i = 0; args[i] != NULL; i++){
        if(strcmp(args[i], "&") == 0){
            args[i] = NULL;
            return true;
        }
    }
    return false;
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
        waitpid(id, NULL, 0);
    }
}

// kill zombies and write in file
void on_child_exit(int sig){
    int status;
    pid_t pid;

    while((pid = waitpid(-1, &status, WNOHANG)) > 0){
        FILE *fptr = fopen(filepath, "a");
        if(fptr != NULL){
            printf("done \n");
            fprintf(fptr, "child terminated\n");
            fclose(fptr);
        }
        else {
            fprintf(stderr, "Error opening file\n");
        }
        cwd();
    }
    
}

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
// printig what inside quotes
void echo(){
    int id = fork();
    if(id == 0){
        const char *start = strchr(line, '"');
        if(start == NULL){
            printf("Not found starting quote\n");
            exit(0);
        }
        start++;
        const char *end = strchr(start, '"');
        if(end == NULL){
            printf("Not found ending quote\n");
            exit(0);
        }
        for (const char *ptr = start; ptr < end; ptr++) {
            putchar(*ptr);
        }
        printf("\n");
        exit(0);
    }
    else{
        waitpid(id, NULL, 0);
    }
}

// making variables
void export(){
    // Find the position of '='
    const char *equal_sign = strchr(line, '=');
    if (equal_sign == NULL) {
        printf("Invalid export string\n");
        return;
    }

    // Find the position of the first space before '='
    const char *space_before_equal = equal_sign;
    while (space_before_equal > line && *(space_before_equal - 1) != ' ') {
        space_before_equal--;
    }

    // Copy the variable name (before '=' until the first space)
    int variable_length = equal_sign - space_before_equal;
    strncpy(variables[idx], space_before_equal, variable_length);
    variables[idx][variable_length] = '\0';

    // Copy the value (after '=')
    strcpy(values[idx], equal_sign + 1);
    int value_length = strlen(values[idx]);
    if (values[idx][value_length - 1] == '\n') {
        values[idx][value_length - 1] = '\0';
    }
    // printf("%s", values[idx]);
    idx++;
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
    if(check_background_process()){
        if (id == 0){
            execvp(args[0], args);
            printf("command not found\n");
            exit(0);
        }
    }
    else{
        if (id == 0){
            execvp(args[0], args);
            printf("command not found\n");
            exit(0);
        }
        else{
            waitpid(id, NULL, 0);
        }
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
    signal(SIGCHLD, on_child_exit);
    setup_environment();
    shell();
    return 0;
}