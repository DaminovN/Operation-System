#ifndef _NICEWEB_LOCAL_H_
#define _NICEWEB_LOCAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BUF_SIZE 1024
#define SERVER_PORT 25665
#define MULTIPLEX_SIZE 1000

#define HANDLE_ERROR(eval) \
    if (eval < 0) \
    { \
        time_t rawtime; \
        struct tm * timeinfo; \
        time(&rawtime); \
        timeinfo = localtime(&rawtime); \
        printf("[%s] ", asctime(timeinfo)); \
        perror("eval"); \
        exit(EXIT_FAILURE); \
    }

#define ASSIGN_HANDLE_ERROR(dst, eval) \
    if ((dst = eval) < 0) \
    { \
        time_t rawtime; \
        struct tm * timeinfo; \
        time(&rawtime); \
        timeinfo = localtime(&rawtime); \
        printf("[%s] ", asctime(timeinfo)); \
        perror("eval"); \
        exit(EXIT_FAILURE); \
    }

#endif
