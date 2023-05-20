#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ipc.h"
#include "server.h"
#include "utils.h"

#ifndef OUTPUTFILE_TEMPLATE
#define OUTPUTFILE_TEMPLATE "../checker/output/out-XXXXXX"
#endif

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

	FILE *log = fopen("server.log", "wa");
	if (log == NULL) {
		perror("Error opening log file");
		return 1;
	}

	/* TODO - Implement server connection */
	int listen_fd = create_socket();
	DIE(listen_fd < 0, "error creating listen socket\n");


	while(1) {

		/* TODO - get message from client */
		/* TODO - parse message with parse_command and populate lib */
		/* TODO - handle request from client */
		ret = lib_run(&lib);
	}

	return 0;
}
