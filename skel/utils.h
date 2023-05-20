
/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef UTILS_H_
#define UTILS_H_ 1

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

extern FILE *logger;

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion)	{					\
			fprintf(logger, "%s: %s",				\
				call_description, strerror(errno));	\
			exit(EXIT_FAILURE);				\
		}							\
	} while (0)

void populate_sockaddr_unix(struct sockaddr_un *addr, const char socket_path[]); 

#ifdef __cplusplus
}
#endif

#endif  /* UTILS_H_ */
