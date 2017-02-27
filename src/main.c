#include <stdio.h>
#include <getopt.h>
#include <signal.h>
#include <netinet/in.h>
#include <unistd.h>
#include "dbg.h"
#include "util.h"
#include "epoll.h"
#include "http_request.h"
#include "timer.h"

#define CONF "yas.conf"
#define PROGRAM_VERSION "0.1"

extern struct epoll_event *events;

static const struct option long_options[] = {
    {"help", no_argument, NULL, '?'},
    {"version", no_argument, NULL, 'V'},
    {"conf", required_argument, NULL, 'c'},
    {NULL, 0, NULL, 0}
};

static void usage(){
    fprintf(
            stderr,
            " YAS [option]... \n"
            " -c|--conf <config file> \n"
            " -?|-h|--help \n"
            " -V|--version"
    );
}

int main(int argc, char* argv[]) {
    printf("Hello, World!\n");
    printf("Hello, Herve!\n");

    int rc;
    int opt = 0;
    int options_index = 0;
    char* conf_file = CONF;

    // parse args
     if (argc == 1) {
         usage();
         return 0;
     }

    while ( (opt=getopt_long(argc, argv, "Vc:?h",
    long_options, &options_index)) != EOF ) {
        switch (opt) {
            case 'c':
                conf_file = optarg;
                break;
            case 'V':
                printf(PROGRAM_VERSION"\n");
                return 0;
            case ':':
            case 'h':
            case '?':
                usage();
                return 0;
        }
    }

    debug("conf_file %s", conf_file);

    if (optind < argc) {
        log_err("non-option ARGV-elements: ");
        while (optind < argc)
            log_err("%s ", argv[optind++]);
        return 0;
    }

    // read conf
    char conf_buf[BUFLEN];
    yas_conf_t cf;
    rc = read_conf(conf_file, &cf, conf_buf, BUFLEN);
    check(rc == ZV_CONF_OK, "read conf err");

    // Server write to closed fd will case system send signal to this process, then exit
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE, &sa, NULL)) {
        log_err("install sigal handler for SIGPIPE failed");
        return 0;
    }

    // init listen socket
    int listenfd;
    struct sockaddr_in clientaddr;
    // initialize clientaddr and inlen to solve "accept Invalid argument" bug
    socklen_t inlen = 1;
    memset(&clientaddr, 0, sizeof(struct sockaddr_in));

    listenfd = open_listenfd(cf.port);
    rc = make_socket_non_blocking(listenfd);
    check(rc == 0, "make_socket_non_blocking");


    // create epoll and add listenfd to ep
    int ep_fd = yas_epoll_create(0);
    struct epoll_event event;
    // init a request
    yas_http_request_t *request = (yas_http_request_t *)malloc(sizeof(yas_http_request_t));
    yas_init_request_t(request, listenfd, ep_fd, &cf);
    // set data ptr to request
    event.data.ptr = (void *)request;
    event.events = EPOLLIN | EPOLLET;
    yas_epoll_add(ep_fd, listenfd, &event);

    // create thread pool
//    zv_threadpool_t *tp = threadpool_init(cf.thread_num);
//    check(tp != NULL, "threadpool_init error");


    // init timer
    yas_timer_init();

    log_info("yas started.");
    int n;
    int i, fd;
    int time;
    // intend to be endless loop
    int loop = 1;

    // event loop
    while (loop) {
        time = yas_find_timer();
        debug("wait time = %d", time);

        // block here util something happens, return number
        n = yas_epoll_wait(ep_fd, events, MAXEVENTS, time);
        // handler expires
        yas_handle_expire_timers();
        
        // traverse events
        for (i = 0; i < n; i++) {
            yas_http_request_t *r = (yas_http_request_t *)events[i].data.ptr;
            fd = r->fd;

            // if this event is on listen fd, means we have a new connection
            if (listenfd == fd) {
                int infd;
                while(1) {
                    infd = accept(listenfd, (struct sockaddr *)&clientaddr, &inlen);
                    if (infd < 0) {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                            /* we have processed all incoming connections */
                            break;
                        } else {
                            log_err("accept");
                            break;
                        }
                    }

                    rc = make_socket_non_blocking(infd);
                    check(rc == 0, "make_socket_non_blocking");
                    log_info("new connection fd %d", infd);

                    yas_http_request_t *request = (yas_http_request_t *)malloc(sizeof(yas_http_request_t));
                    if (request == NULL) {
                        log_err("malloc(sizeof(zv_http_request_t))");
                        break;
                    }

                    yas_init_request_t(request, infd, ep_fd, &cf);
                    event.data.ptr = (void *)request;
                    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;

                    yas_epoll_add(ep_fd, infd, &event);
                    yas_add_timer(request, TIMEOUT_DEFAULT, yas_http_close_conn);
                }   // end of while of accept
            } else { // else means events on established sockets
                if ((events[i].events & EPOLLERR) ||
                    (events[i].events & EPOLLHUP) ||
                    (!(events[i].events & EPOLLIN))) {
                    log_err("epoll error fd: %d", r->fd);
                    close(fd);
                    continue;
                }

                log_info("new data from fd %d", fd);
                //rc = threadpool_add(tp, do_request, events[i].data.ptr);
                //check(rc == 0, "threadpool_add");

//                do_request(events[i].data.ptr);
            }
        }
    }

    /*
    if (threadpool_destroy(tp, 1) < 0) {
        log_err("destroy threadpool failed");
    }
    */

    printf("Before return\n");
    return 0;
}