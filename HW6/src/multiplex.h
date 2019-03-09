#ifndef _NICEWEB_MULTIPLEX_H_
#define _NICEWEB_MULTIPLEX_H_

int multiplex();

int mx_add(int mxfd, int fd);

int mx_wait(int mxfd, int* read_fds, size_t max_events);

int set_nonblock(int sockfd);

#endif
