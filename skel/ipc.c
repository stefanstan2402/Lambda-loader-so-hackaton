#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "ipc.h"
#include "utils.h"

const char socket_path[] = "../golden_gate";

int create_socket()
{
	// create unix socket
	int fd;
	fprintf(stdout, "Creating socket...\n");
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
	printf("Sending %zu bytes...\n", len);
	size_t total_bytes_sent = 0;
	size_t curr_bytes_sent = 0;

	while (total_bytes_sent < len) {
		curr_bytes_sent = send(fd, buf + total_bytes_sent, len - total_bytes_sent, 0);
		printf("Trimiseram atatia %zu bytes...\n", curr_bytes_sent);	
		if (curr_bytes_sent < 0) {
			perror("send socket");
			return -1;
		}

		if (curr_bytes_sent == 0) {
			return total_bytes_sent;
		}

		total_bytes_sent += curr_bytes_sent;
	}

	return total_bytes_sent;
}

ssize_t recv_socket(int fd, char *buf, size_t len)
{
	printf("Receiving %zu bytes...\n", len);
	size_t total_bytes_received = 0;
	size_t curr_bytes_received = 0;
	
	while(len > total_bytes_received) {
		curr_bytes_received = recv(fd, buf + total_bytes_received, len - total_bytes_received, 0);
		if (curr_bytes_received == -1) {
			perror("recv");
			return -1;
		}
		if (curr_bytes_received == 0) {
			fprintf(stderr, "EOF\n");
			return 0;
		}
		total_bytes_received += curr_bytes_received;
		if (total_bytes_received == len) {
			break;
		}
	}
	return total_bytes_received;
}

void close_socket(int fd)
{
	close(fd);
}

