#include "utils.h"

void populate_sockaddr_unix(struct sockaddr_un *addr, const char socket_path[]) {
	memset(addr, 0, sizeof((*addr)));
	addr->sun_family = AF_UNIX;
    
	snprintf(addr->sun_path, strlen(socket_path) + 1, "%s", socket_path);
}