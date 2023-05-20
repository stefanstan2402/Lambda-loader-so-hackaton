#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "ipc.h"
#include "server.h"
#include "utils.h"

#ifndef OUTPUTFILE_TEMPLATE
#define OUTPUTFILE_TEMPLATE "../checker/output/out-XXXXXX"
#endif

#define BUFLEN 1024

FILE *logger = NULL;
const char socket_path[] = "../golden_gate";

static int lib_prehooks(struct lib *lib)
{
	return 0;
}

static int lib_load(struct lib *lib)
{
	return 0;
}

static int lib_execute(struct lib *lib)
{
	return 0;
}

static int lib_close(struct lib *lib)
{
	return 0;
}

static int lib_posthooks(struct lib *lib)
{
	return 0;
}

static int lib_run(struct lib *lib)
{
	int err;

	err = lib_prehooks(lib);
	if (err)
		return err;

	err = lib_load(lib);
	if (err)
		return err;

	err = lib_execute(lib);
	if (err)
		return err;

	err = lib_close(lib);
	if (err)
		return err;

	return lib_posthooks(lib);
}

static int parse_command(const char *buf, char *name, char *func, char *params)
{
	return sscanf(buf, "%s %s %s", name, func, params);
}

int main(void)
{
	int ret;
	struct lib lib;

	FILE *logger = fopen("server.log", "wa");
	if (logger == NULL) {
		printf("Error opening log file\n");
		perror("Error opening log file");
		return 1;
	}
	setvbuf(logger, NULL, _IONBF, 0);

	fprintf(logger, "server started\n");
	// fflush(logger);
	remove(socket_path);

	/* TODO - Implement server connection */
	int listen_fd = create_socket();
	DIE(listen_fd < 0, "error creating listen socket\n");

	struct sockaddr_un addr;
	populate_sockaddr_unix(&addr, socket_path);

	ret = bind(listen_fd, (struct sockaddr *) &addr, sizeof(addr));
	DIE(ret < 0, "bind");

	ret = listen(listen_fd, 10);
	DIE(ret < 0, "listen");

	// use epoll to handle multiple clients
	int epoll_fd = epoll_create1(0);
	DIE(epoll_fd < 0, "epoll_create1");
	struct epoll_event event, events[100];

	event.events = EPOLLIN;
	event.data.fd = listen_fd;

	ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event);
	DIE(ret < 0, "epoll_ctl");

	//number of (file descriptors
	int nfds = 0;
	int client_fd;
	
	while(1) {

		/* TODO - get message from client */

		// monitor readfds for readiness for reading
		if ((nfds = epoll_wait(epoll_fd, events, 100, -1)) < 0) {
			perror("epoll_wait");
			return -1;
		}

		// Some sockets are ready. Examine readfds
		for (int i = 0; i < nfds; i++) {
			if (events[i].data.fd == listen_fd) {
				// new client connection
				client_fd = accept(listen_fd, NULL, NULL);
				DIE(client_fd < 0, "accept");

				event.events = EPOLLIN;
				event.data.fd = client_fd;

				ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
				DIE(ret < 0, "epoll_ctl");
			} else {
				// client sent message
				char buf[BUFLEN];
				memset(buf, 0, BUFLEN);

				ret = recv_socket(events[i].data.fd, buf, BUFLEN);
				DIE(ret < 0, "recv");

				if (ret == 0) {
					// client disconnected
					ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
					DIE(ret < 0, "epoll_ctl");
					close_socket(events[i].data.fd);
				} else {
					// client sent message
					char name[BUFLEN], func[BUFLEN], params[BUFLEN];
					memset(name, 0, BUFLEN);
					memset(func, 0, BUFLEN);
					memset(params, 0, BUFLEN);

					ret = parse_command(buf, name, func, params);
					DIE(ret < 0, "parse_command");
					fprintf(logger, "Received command: %s %s %s\n", name, func, params);
					// TODO - parse message with parse_command and populate lib
					// TODO - handle request from client
					ret = lib_run(&lib);
				}
			}
		}

		/* TODO - parse message with parse_command and populate lib */
		/* TODO - handle request from client */
		ret = lib_run(&lib);
	}

	return 0;
}
