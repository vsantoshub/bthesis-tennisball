/* 
 * File:	cmd.c
 * Author:   Victor Santos (viic.santos@gmail.com)
 * Comment:
 * 
 */

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>

#include "application.h"
#include "debug.h"

struct cmd_entry {
    int (* callback)(int, char **);
    char name[16];
    char * const usage;
    char * const desc;
};

extern const struct cmd_entry cmd_tab[];

static int show_cmd_usage(const char * cmd)
{
    struct cmd_entry * ep;

    ep = (struct cmd_entry *)cmd_tab;

    while (*ep->name != '\0') {
        if (strcmp(cmd, ep->name) == 0) {
            printf("usage: %s %s\n", ep->name,  ep->usage);
            return 0;
        }
        ep++;
    }

    return -1;
}

/* 
 * Command implementation
 */


int cmd_null(int argc, char ** argv)
{
    return 0;
}

int cmd_help(int argc, char ** argv)
{
    struct cmd_entry * ep;

    if (argc > 2)
        return show_cmd_usage(argv[0]);

    if (argc > 1) {
        ep = (struct cmd_entry *)cmd_tab;
        while (*ep->name != '\0') {
            if (strcmp(argv[1], ep->name) == 0) {
                printf("%s.\n", ep->desc);
                printf("usage: %s %s\n", ep->name,  ep->usage);
                return 0;
            }
            ep++;
        }
        return -1;
    }

    printf("\n");
    ep = (struct cmd_entry *)cmd_tab;
    while (*ep->name != '\0') {
        printf(" %-12s %s\n", ep->name, ep->desc);
        ep++;
    }

    return 0;
}

int cmd_quit(int argc, char ** argv)
{
    printf("Bye...\n");
    kill(app_pid, SIGQUIT);
    return 0;
}

int cmd_ver(int argc, char ** argv)
{

    fprintf(stderr, "%s %d.%d (built on " __DATE__ " " __TIME__ ")\n", APP_NAME, APP_VERSION_MAJOR, APP_VERSION_MINOR);

    return 0;
}

 
/*
 * Command table
 */

const struct cmd_entry cmd_tab[] = {
    { cmd_help, "help", (char *)"",
        (char *)"mostrar essa mensagem de help"},
    { cmd_quit, "quit", (char *)"",
        (char *)"encerrar a aplicacao"},
    { cmd_ver, "ver", (char *)"",
        (char *)"exibir a versao do aplicativo"},
    { NULL, "",  NULL, NULL}
};

extern "C" int cmd_exec(int argc, char * argv[])
{
    struct cmd_entry * ep;
    char * cmd;
    int n;

    cmd = argv[0];

    if ((n = strlen(cmd)) == 0) {
        return 0;
    }

    ep = (struct cmd_entry *)cmd_tab;

    while (*ep->name != '\0') {
        if (strcmp(cmd, ep->name) == 0) {
            return ep->callback(argc, argv);
        }
        ep++;
    }

    printf("Command invalid. Type 'help'.\n");

    return 0;
}

/* Strip whitespace from the start and end of STRING.  Return a pointer
   into STRING. */
static char * cmd_stripwhite(char * string)
{
    register char *s, *t;

    for (s = string; isspace(*s); s++);

    if (*s == 0)
        return (s);

    t = s + strlen (s) - 1;
    while (t > s && isspace(*t))
        t--;
    *++t = '\0';

    return s;
}

extern "C" int cmd_line_parse(char * s, char ** argv, int argmax)
{
    char * cp;
    int bracket;
    int quote;
    int n;
    int c;


    if (!s)
        return 0;

    if ((cp = cmd_stripwhite(s)) == '\0') {
        return 0;
    }

    if ((*cp == '\n') || (*cp == '\r'))
        return 0;

    bracket = 0;
    quote = 0;

    for (n = 0; (c = *cp) && (n < argmax); ) {
        c = *cp;
        /* Remove lead blanks */
        if (isspace(c)) {
            cp++;
            continue;
        }
        if (c == '(') {
            bracket++;
            cp++;
        }
        if (c == '"') {
            cp++;
            if (quote) {
                quote = 0;
                continue;
            }
            quote = 1;
        }
        if ((c == ')') && (bracket)) {
            bracket = 0;
            continue;
        }

        argv[n++] = cp;

        if (bracket) {
            for(; (c = *cp); cp++) {
                if (c == '(')
                    bracket++;
                if (c == ')') {
                    bracket--;
                    if (!bracket)
                        break;
                }
            } 
        } else {
            if (quote) {
                for (; (c = *cp) && (c != '"'); cp++);
                quote = 0;
            } else {
                for (; (c = *cp); cp++) {
                    if (isspace(c)) {
                        break;
                    }
                    if (c == '"') {
                        quote = 1;
                        break;
                    }
                    if (c == '(') {
                        bracket = 1;
                        break;
                    }
                }
            }
        }
        if (c)
            *cp++ = '\0';
    }

    return n;
}

