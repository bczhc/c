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

    for (int j = 0; j < 100; ++j) {
        char buf[256];
        int eolIndex = -1;
        for (int i = 0; i < 256; ++i) {
            read(clientFD, buf + i, 1);
            if (buf[i] == '\n') {
                eolIndex = i;
                break;
            }
        }
        if (eolIndex == -1) {
            fprintf(stderr, "Data received has no newLine mark\n");
            continue;
        }
        buf[eolIndex] = '\0';

        printf("%s\n", buf);
        int responseLength = 7 + eolIndex + 1;
        char response[responseLength];
        strcpy(response, "hello, ");
        strncpy(response + 7, buf, eolIndex);
        response[responseLength - 1] = '\n';

        write(clientFD, response, responseLength);
    }

    close(clientFD);
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
        int clintFD = 0;
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
