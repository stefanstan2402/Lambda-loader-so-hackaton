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
	if (lib->libname == NULL) {
		fprintf(logger, "libname is NULL\n");
		return 1;
	}
	return 0;
}

static int lib_load(struct lib *lib)
{
	// load library from lib.libname
	lib->handle = dlopen(lib->libname, RTLD_NOW);
	
	if (lib->handle == NULL) {
		if (lib->funcname == NULL) {
			printf("Error: %s could not be executed.\n", lib->libname);
		} else if (lib->filename == NULL) {
			printf("Error: %s %s could not be executed.\n", lib->libname, lib->funcname);
		} else {
			printf("Error: %s %s %s could not be executed.\n", lib->libname, lib->funcname, lib->filename);
		}
		fprintf(logger, "dlopen: %s\n", dlerror());
		return 1;
	}

	return 0;
}

static int lib_execute(struct lib *lib)
{
	int err = 0;

	if (lib->filename == NULL) {
		// call lib.run
		if (lib->run == NULL) {
			if (lib->funcname == NULL) {
				lib->funcname = malloc(4);
				strcpy(lib->funcname, "run");
			}

			lib->run = dlsym(lib->handle, lib->funcname);
			if (!lib->run) {
				printf("Error: %s %s %s could not be executed.\n", lib->libname, lib->funcname, lib->filename);
				fprintf(logger, "dlsym: %s\n", dlerror());
				err = 1;
			}
		}

		lib->run();
	} else {
		// call lib.p_run
		if (lib->p_run == NULL) {
			lib->p_run = dlsym(lib->handle, lib->funcname);
			if (!lib->p_run) {
				printf("Error: %s %s %s could not be executed.\n", lib->libname, lib->funcname, lib->filename);
				fprintf(logger, "dlsym: %s\n", dlerror());
				err = 1;
			}
		}

		lib->p_run(lib->filename);
	}

	return err;
}

static int lib_close(struct lib *lib)
{
	// close library
	dlclose(lib->handle);

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

	event.events = EPOLLIN | EPOLLOUT;
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

				event.events = EPOLLIN ;
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
					if (ret < 0) {
						fprintf(logger, "Error parsing command\n");
						close_socket(events[i].data.fd);
						continue;
					}
					fprintf(logger, "Received command: %s %s %s\n", name, func, params);

					struct lib *lib = malloc(sizeof(struct lib));
					lib->libname = malloc(strlen(name) + 1);
					strcpy(lib->libname, name);

					if (strlen(func) > 0) {
						lib->funcname = malloc(strlen(func) + 1);
						strcpy(lib->funcname, func);
					}
					
					if (strlen(params) > 0) {
						lib->filename = malloc(strlen(params) + 1);
						strcpy(lib->filename, params);
					}
			
					lib->outputfile = malloc(strlen(OUTPUTFILE_TEMPLATE) + 1);
					// use mkstemp to create a unique file name
					strcpy(lib->outputfile, OUTPUTFILE_TEMPLATE);
					int fd = mkstemp(lib->outputfile);

					pid_t pid = fork();
					int status;

					switch (pid) {
						case -1:
							fprintf(logger, "Error forking\n");
							break;
						case 0:
							// child process
							dup2(fd, STDOUT_FILENO);
							setvbuf(stdout, NULL, _IONBF, 0);
							close(fd);
							ret = lib_run(lib);
							if (ret < 0) {
								fprintf(logger, "Error running command from client\n");
							}
							exit(0);
							break;
						default:
							// parent process
							waitpid(pid, &status, 0);
							ret = send_socket(events[i].data.fd, lib->outputfile, strlen(lib->outputfile));
							if (ret < 0) {
								fprintf(logger, "Error sending response to client\n");
							}
							close(fd);
							break;
					}
				}
			}
		}
	}

	return 0;
}
