#define OS_UNIX 1
#define OS_MAC  2

#if defined(__unix__)
    #define OS OS_UNIX
#elif defined(__APPLE__)
    #define OS OS_MAC
#endif

#if OS == OS_UNIX
    #include <fcntl.h>
    #include <sys/epoll.h>
    #include <sys/types.h>
    #include <unistd.h>
#elif OS == OS_MAC
    #include <fcntl.h>
    #include <sys/types.h>
    #include <sys/event.h>
    #include <sys/time.h>
    #include <unistd.h>
#endif

#include "local.h"

#if OS == OS_UNIX
static struct epoll_event ev, evlist[MULTIPLEX_SIZE];
#elif OS == OS_MAC
static struct kevent ev_set,  evlist[MULTIPLEX_SIZE];
#endif

int multiplex()
{
    int mxfd;
#if OS == OS_UNIX
    ev.events = EPOLLIN | EPOLLET;

    if ((mxfd = epoll_create(MULTIPLEX_SIZE)) < 0)
    {
        return -1;
    }
#elif OS == OS_MAC
    if ((mxfd = kqueue()) < 0)
    {
        return -1;
    }
#endif
    return mxfd;
}

int mx_add(int mxfd, int fd)
{
#if OS == OS_UNIX
    ev.data.fd = fd;
    if (epoll_ctl(mxfd, EPOLL_CTL_ADD, fd, &ev) < 0)
    {
        return -1;
    }
#elif OS == OS_MAC
    EV_SET(&ev_set, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    if (kevent(mxfd, &ev_set, 1, NULL, 0, NULL) < 0)
    {
        return -1;
    }
#endif
    return 0;
}

int mx_wait(int mxfd, int* read_fds, size_t max_events)
{
    if (max_events > MULTIPLEX_SIZE)
    {
        return -1;
    }
    int events_count;
#if OS == OS_UNIX
    if ((events_count = epoll_wait(mxfd, evlist, max_events, -1)) < 0)
    {
        return -1;
    }
    for (size_t i = 0; i < events_count; ++i)
    {
        read_fds[i] = evlist[i].data.fd;
    }
#elif OS == OS_MAC
    if ((events_count = kevent(mxfd, NULL, 0, evlist, max_events, NULL)) < 0)
    {
        return -1;
    }
    for (size_t i = 0; i < events_count; ++i)
    {
        read_fds[i] = evlist[i].ident;
    }
#endif
    return events_count;
}

int set_nonblock(int sockfd)
{
#if OS == OS_UNIX || OS == OS_MAC
    return fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK);
#endif
}
