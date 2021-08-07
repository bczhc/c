#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

struct ThreadArgs {
    int clientFD;
};


void *accept_thread(void *arg) {
    struct ThreadArgs *threadArgs = (struct ThreadArgs *) arg;
    int clientFD = threadArgs->clientFD;

    char head[5];
    read(clientFD, head, sizeof(head));

    if (strncmp(head, "bczhc", 5) == 0) {
        printf("Match head!\n");
        char buf[100];
        size_t readLen = read(clientFD, buf, 100);
        if (readLen > 0) {
            buf[readLen] = '\0';
            printf("Received: %s\n", buf);
        } else {
            fprintf(stderr, "Read error\n");
        }
        close(clientFD);

    } else {
        printf("Mismatched head...\n");
        close(clientFD);
    }

    free(threadArgs);
    return NULL;
}

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(1234);
    serverAddr.sin_family = AF_INET;

    if (bind(fd, (const struct sockaddr *) &serverAddr, sizeof(serverAddr)) != 0) {
        fprintf(stderr, "Failed to bind: %i\n", errno);
        exit(1);
    }

    if (listen(fd, 1) != 0) {
        fprintf(stderr, "Failed to listen\n");
        exit(1);
    }

    for (int i = 0; i < 10; ++i) {
        int clintFD;
        if ((clintFD = accept(fd, NULL, NULL)) == -1) {
            fprintf(stderr, "Failed to accept\n");
            continue;
        }

        pthread_t thread;
        struct ThreadArgs *args = (struct ThreadArgs *) malloc(sizeof(struct ThreadArgs));
        args->clientFD = clintFD;
        pthread_create(&thread, NULL, accept_thread, args);
    }

    close(fd);

    return 0;
}
