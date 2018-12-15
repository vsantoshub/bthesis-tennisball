/* 
 *
 * File:	lookup.c
 * Module:
 * Project:	libconf
 *  Author:   Victor Santos (viic.santos@gmail.com)
 * Target:
 * Comment:
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "conf.h"

int conf_lookup(char * pathname, const char ** searchpath, 
				const char ** filename)
{
	char * fn;
	char * sp;
	struct stat buf;
	int i;
	int j;

	if ((pathname == NULL) || (searchpath == NULL) || (filename == NULL))
		return -1;

	for (i = 0; filename[i] != NULL; i++) {
		for(fn = (char *)filename[i]; (*fn != '\0') && (*fn != '/') 
				&& (isspace(*fn)); fn++);
		if (*fn == '\0')
			continue;

		for (j = 0; searchpath[j] != NULL; j++) {

			for(sp = (char *)searchpath[j]; (*sp != '\0') && 
				(isspace(*sp)); sp++);

			/* very basic shell expansion */
			if (*sp == '~') {
				strcpy(pathname, getenv("HOME"));
			} else 
				strcpy(pathname, sp);
			strcat(pathname, "/");
			strcat(pathname, fn);

			if (stat(pathname, &buf) == 0) {

				if (S_ISREG(buf.st_mode))
					return 0;

			}
		}
	}

	return -1;
}

