#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#define MAX_LINE 80

void execute_command(char *cmdline) {
    char *args[MAX_LINE/2+1];
    int argc = 0;
    char *token = strtok(cmdline, " ");
    while (token != NULL && argc < MAX_LINE/2) {
        args[argc++] = token;
        token = strtok(NULL, " ");
    }
    args[argc] = NULL;

    if (argc > 0) {
        int background = 0;
        if (argc > 0 && strcmp(args[argc-1], "&") == 0) {
            background = 1;
            args[argc-1] = NULL;
            argc--;
        }

        pid_t pid = fork();//fork a new thread
        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        } 
        else if (pid == 0) {
            if (execvp(args[0], args) < 0) {
                perror("Exec failed");
                exit(1);
            }
        } 
        else {
            if (!background) {
                waitpid(pid, NULL, 0);
            }
        }
    }
}

int main(void) {
    char *args[MAX_LINE/2+1];
    char *history[36];
    int should_run = 1;
    int index = 0, flag = 0;

    while (should_run) {
        printf("osh>");
        fflush(stdout);
        char temp[24] = {0}; 
        int i = 0, argc = 0;
        char c;

        while ((c = getchar()) != '\n') {
            if (c == ' ') {
                char *word = (char*)malloc(sizeof(char)*(i + 1));
                if (!word) exit(1);
                strcpy(word, temp);
                word[i] = '\0';
                args[argc++] = word;
                i = 0;
            } else {
                temp[i++] = c;
            }
        }
        if (i > 0) { //last arg
            char *word = (char*)malloc(sizeof(char)*(i + 1));
            if (!word) exit(1);
            strcpy(word, temp);
            word[i] = '\0';
            args[argc++] = word;
        }

        //copy to history arr
        int sign = 0;
        history[index] = (char*)malloc(sizeof(char) * 100);
        if (!history[index]) exit(1);
        for (int i = 0; i < argc; i++) {
            for (int j = 0; j < strlen(args[i]); j++) {
                history[index][sign++] = args[i][j];
            }
            history[index][sign++] = ' ';
        }
        history[index][sign - 1] = '\0';
        if (index == 35) {
            flag = 1;
            index = 0;
        } else {
            index++;
        }

        // execute
        if (argc > 0) {
            if (strcmp(args[0], "exit") == 0) {//exit
                should_run = 0;
            } 
            else if (strcmp(args[0], "history") == 0) {//history
                if (flag == 0) {
                    for (int i = index - 1; i >= 0; i--) {
                        printf("%d %s\n", (i+1), history[i]);
                    }
                } 
                else {
                    for (int i = 0; i < 35; i++) {
                        printf("%d %s\n", 35 - i, history[(index + 35 - i) % 36]);
                    }
                }
            } 
            else if (strcmp(args[0], "!!") == 0) {//!!
                int last_index;
                if (flag == 0 && index == 1) {
                    printf("No such commands in history.\n");
                    continue;
                }
                if (flag == 0)
                    last_index = index - 2;
                else
                    last_index = (index - 2 + 36) % 36;
                char temp[100];
                strcpy(temp, history[last_index]);
                printf("%s\n", temp);
                if (strcmp(temp, "history") == 0) {
                    if (flag == 0) {
                        for (int i = index - 1; i >= 0; i--) {
                            printf("%d %s\n", (i+1), history[i]);
                        }
                    } else {
                        for (int i = 0; i < 35; i++) {
                            printf("%d %s\n", 35 - i, history[(index + 35 - i) % 36]);
                        }
                    }
                } 
                else if (strcmp(temp, "exit") == 0) {
                    should_run = 0;
                } 
                else {
                    execute_command(temp);
                }
            } 
            else if (args[0][0] == '!' && args[0][1] != '\0' && args[0][1] != '!') {//!n
                int cmd_num = atoi(&args[0][1]);
                int target_index;
                if (flag == 0) {
                    if (cmd_num < 1 || cmd_num > index) {
                        printf("No such command in history.\n");
                        continue;
                    }
                    target_index = cmd_num - 1;
                } 
                else {
                    if (cmd_num < 1 || cmd_num > 35) {
                        printf("No such command in history.\n");
                        continue;
                    }
                    int offset = 35 - cmd_num;
                    target_index = (index - 1 - offset + 36) % 36;
                }
                char temp[100];
                strcpy(temp, history[target_index]);
                printf("%s\n", temp);
                execute_command(temp);
            } 
            else {//other commands
                char cmdline[100] = "";
                for (int i = 0; i < argc; i++) {
                    strcat(cmdline, args[i]);
                    if (i < argc - 1) strcat(cmdline, " ");
                }
                execute_command(cmdline);
            }
        }
        for (int i = 0; i < argc; i++) {
            free(args[i]);
        }
    }
    for (int i = 0; i < (flag == 0 ? index : 36); i++) {
        free(history[i]);
    }
    return 0;
}