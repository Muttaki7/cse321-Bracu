#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    char buffer[256];
    while (1) {
        printf("Enter text (-1 to stop): ");
        fgets(buffer, sizeof(buffer), stdin);

        if (strncmp(buffer, "-1", 2) == 0)
            break;

        write(fd, buffer, strlen(buffer));
    }

    close(fd);
    return 0;
};
