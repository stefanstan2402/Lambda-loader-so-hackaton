#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "ipc.h"
#include "utils.h"

const char socket_path[] = "golden_gate";

int create_socket()
{
	// create unix socket
	int fd;
	struct sockaddr_un addr;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket");
		return -1;
	}

	return fd;
}

int connect_socket(int fd)
{
	// connect to unix socket
	int rc = 0;
	struct sockaddr_un addr;
	populate_sockaddr_unix(&addr, socket_path);

	rc = connect(fd, (struct sockaddr *)&addr, sizeof(addr));	

	return rc;
}

ssize_t send_socket(int fd, const char *buf, size_t len)
{
	return -1;
}

ssize_t recv_socket(int fd, char *buf, size_t len)
{
	return -1;
}

void close_socket(int fd)
{
}

