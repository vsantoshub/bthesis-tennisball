/* 
 * File:	shell.c
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
#include <time.h>
#include <ctype.h>
#include <signal.h>

#include "cmd.h"
#include "debug.h"

char * readline(const char * prompt, char * buf, int max)
{
	int n;

	printf("%s", prompt);
	fflush(stdout);

	n = read(STDIN_FILENO, buf, max);

	if (n <= 0) {
		free(buf);
		return NULL;
	}

	buf[n] = '\0';

	return buf;
}

#define ARGMAX 16
#define LINEMAX 128

int shell(void)
{
	char line[LINEMAX];
	char * argv[ARGMAX];
	int argc;
	int n;
	time_t now_t;
	struct tm * now_st;
	char s[64];

	printf("\n");

	for (;;) {

		now_t = time(NULL);
		now_st = localtime(&now_t);

		strftime(s, sizeof(s), "[%Y-%m-%d %H:%M:%S]: ", now_st);

		readline(s, line, LINEMAX);

        //printf("len(%d):\n[%s]\n", (int)strlen(line), line);
        //DBG_DUMP(DBG_TRACE, line, strlen(line));
		argc = cmd_line_parse(line, argv, ARGMAX);
		if (argc == 0) {
			continue;
		}

		n = cmd_exec(argc, argv);
		if (n > 0)
			return 0;

		if (n < 0)
			printf("Error: %d\n", n);
		else
			printf("\n");
	}
	return 0;
}

