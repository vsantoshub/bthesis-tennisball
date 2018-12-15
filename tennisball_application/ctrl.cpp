/* 
 * File:	ctrl.c
 * Author:   Victor Santos (viic.santos@gmail.com)
 * Comment:
 */

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <signal.h>
#include <pthread.h>
#include <fcntl.h>

#include "cmd.h"
#include "debug.h"

static int ctrl_fifo;

static pthread_t ctrl_thread;

static char ctrl_path[128];

#define ARGMAX 8

static int ctrl_task(char * path)
{       
	ssize_t n;
	char msg[512];
	sigset_t mask;
	char * argv[ARGMAX];
	int argc;
	int fd;

	/* block signals */
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGQUIT);
	sigaddset(&mask, SIGABRT);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	DBG(DBG_TRACE, "PID: %d, PPID: %d", getpid(), getppid());

	for (;;) {
		DBG(DBG_INFO, "open(%s)...", path);
		if((fd = open(path, O_RDONLY)) < 0) {
			DBG(DBG_WARNING, "can't open pipe: %s", path);
			return -1;
		}

		ctrl_fifo = fd;

		while ((n = read(fd, msg, 512)) > 0) {

			msg[n] = '\0';
			argc = cmd_line_parse(msg, argv, ARGMAX);

			if (argc == 0) {
				continue;
			}

			DBG(DBG_TRACE, "%s", argv[0]); 

			if (cmd_exec(argc, argv) < 0) {
				DBG(DBG_WARNING, "cmd_exec(%s) fail", argv[0]); 
			}
		}
		if (n < 0) {
			DBG(DBG_WARNING, "read(): n=%d", (int)n); 
			return n;
		}

		close(fd);
		DBG(DBG_INFO, "closed()"); 
	}
	return 0;
}

int ctrl_start(char * path) 
{
//	int fd;
//	struct sockaddr_un addr;

	if (path == NULL)
		return -1;

	/* remove existing file */
	unlink(path);

#if 0
	fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_LOCAL;
	strcpy(addr.sun_path, path);

	if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0) {
		DBG(DBG_WARNING, "can't bind to socket: %s", path);
		return -1;
	}
#endif

	DBG(DBG_TRACE, "mkfifo(%s)...", path);
	if (mkfifo(path, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH) < 0) {
		DBG(DBG_WARNING, "can't create pipe: %s", path);
		return -1;
	}

	strcpy(ctrl_path, path);

	DBG(DBG_TRACE, "pthread_create()...");
	return pthread_create(&ctrl_thread, NULL, 
						  (void * (*)(void *))ctrl_task, 
						  (void *)ctrl_path);
}

int ctrl_stop(void)
{
	close(ctrl_fifo);
	pthread_cancel(ctrl_thread);
	pthread_join(ctrl_thread, NULL);

	unlink(ctrl_path);

	DBG(DBG_TRACE, "bye-bye!");
	return 0;
}

