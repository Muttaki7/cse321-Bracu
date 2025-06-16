#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define BUF_SIZE 1024
#define ARG_LIM 100
#define HIST_LIM 50

char *history[HIST_LIM];
int hist_cnt = 0;

void run(char *cmd);
void handle_pipes(char *cmd);
void handle_redirection(char *args[]);
void execute(char *args[]);
void sig_handler(int signo);
void add_history(char *cmd);
void show_history();
void exec_history(int index);

int main() {
    char input[BUF_SIZE];
    signal(SIGINT, sig_handler);

    while (1) {
        printf("sh> ");
        fflush(stdout);
        
        if (!fgets(input, BUF_SIZE, stdin)) break;
        input[strcspn(input, "\n")] = '\0';
        if (strlen(input) == 0) continue;

        add_history(input);
        
        if (input[0] == '!' && strlen(input) > 1) {
            int idx = atoi(&input[1]);
            exec_history(idx);
            continue;
        }

        char *cmd = strtok(input, ";");
        while (cmd != NULL) {
            run(cmd);
            cmd = strtok(NULL, ";");
        }
    }
    return 0;
}

void run(char *cmd) {
    char *cmds[10];
    int i = 0;
    cmds[i] = strtok(cmd, "&&");
    while (cmds[i] != NULL) {
        i++;
        cmds[i] = strtok(NULL, "&&");
    }
    for (int j = 0; j < i; j++) {
        if (strchr(cmds[j], '|')) {
            handle_pipes(cmds[j]);
        } else {
            char *args[ARG_LIM];
            int k = 0;
            args[k] = strtok(cmds[j], " ");
            while (args[k] != NULL) {
                k++;
                args[k] = strtok(NULL, " ");
            }
            execute(args);
            if (j < i - 1 && access(args[0], X_OK) != 0) {
                break;
            }
        }
    }
}

void execute(char *args[]) {
    pid_t pid = fork();
    if (pid == 0) {
        handle_redirection(args);
        execvp(args[0], args);
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }
}

void handle_redirection(char *args[]) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
        } else if (strcmp(args[i], ">>") == 0) {
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
        } else if (strcmp(args[i], "<") == 0) {
            int fd = open(args[i + 1], O_RDONLY);
            dup2(fd, STDIN_FILENO);
            close(fd);
            args[i] = NULL;
        }
    }
}

void handle_pipes(char *cmd) {
    char *cmds[10];
    int num = 0;
    cmds[num] = strtok(cmd, "|");
    while (cmds[num] != NULL) {
        num++;
        cmds[num] = strtok(NULL, "|");
    }
    int pipes[num - 1][2];
    for (int i = 0; i < num; i++) {
        if (i < num - 1) pipe(pipes[i]);
        pid_t pid = fork();
        if (pid == 0) {
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
                close(pipes[i - 1][0]);
            }
            if (i < num - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
                close(pipes[i][1]);
            }
            for (int j = 0; j < num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            char *args[ARG_LIM];
            int k = 0;
            args[k] = strtok(cmds[i], " ");
            while (args[k] != NULL) {
                k++;
                args[k] = strtok(NULL, " ");
            }
            execvp(args[0], args);
            exit(1);
        }
    }
    for (int j = 0; j < num - 1; j++) {
        close(pipes[j][0]);
        close(pipes[j][1]);
    }
    for (int i = 0; i < num; i++) {
        wait(NULL);
    }
}

void add_history(char *cmd) {
    if (hist_cnt < HIST_LIM) {
        history[hist_cnt] = strdup(cmd);
        hist_cnt++;
    } else {
        free(history[0]);
        for (int i = 1; i < HIST_LIM; i++) {
            history[i - 1] = history[i];
        }
        history[HIST_LIM - 1] = strdup(cmd);
    }
}

void show_history() {
    for (int i = 0; i < hist_cnt; i++) {
        printf("%d %s\n", i + 1, history[i]);
    }
}

void exec_history(int index) {
    if (index > 0 && index <= hist_cnt) {
        run(history[index - 1]);
    }
}

void sig_handler(int signo) {
    if (signo == SIGINT) {
        printf("\nUse 'exit' to quit.\n");
        printf("sh> ");
        fflush(stdout);
    }
}
