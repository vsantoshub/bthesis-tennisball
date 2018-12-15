/* 
 * File:	application.c
 * Author:   Victor Santos (viic.santos@gmail.com)
 * Comment: Startup e Cleanup do aplicativo.
 * 
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
#include <pthread.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "application.h"
#include "ctrl.h"
#include "conf.h"
#include "cmd.h"
#include "server.h"
#include "debug.h"
#include "tasks.h"
#include "relay.h"
#include "monster_shield.h"
#include "servo.h"
#include "http.h"
/*
Public var
*/
int app_log_enabled = 1;

char * app_name;

int app_pid;

char * app_pid_path = (char *)"/var/run/app.pid";

char * app_fifo_name = (char *) "/tmp/app.fifo";

char * app_conf_path = (char *) "./conf_files/app.ini";

int app_nodaemon = 0;            // 0 = rodando em background

/*
 * Configuration profile
 */
BEGIN_SECTION(conf_log)
	DEFINE_INT("habilitado", &app_log_enabled)
END_SECTION

BEGIN_SECTION(conf_root)
	DEFINE_SECTION("log", &conf_log)
END_SECTION
/* *INDENT-ON* */

/*
prototype
*/
int shell(void);


/*
Implementation
*/


/*
Retorna um ponteiro para o inicio do nome do arquivo, desprezando o path.
*/
char * app_extract_fname(char * fname)
{
	char * p;

	if ((p = strrchr(fname, '/')) != NULL)  //procurar pela última ocorrência de '/'
		p++;
	else
		p = fname;  //não existe nenhum '/' (não têm path)

	return p;
}


/*
Cleanup do aplicativo.
*/
void app_cleanup(void)
{
	printf("* Cleanup\n");

	http_stop();

    relay_close();

    monster_shield_close();

    servo_close();

	server_stop();

    tasks_stop();
        
	ctrl_stop();

	unlink(app_pid_path);
}


/*
Handle de signal com função de finalização do processo.
*/
void app_termination_handler(int signum)
{
	printf("\n* Signal %d\n", signum);

	app_cleanup();

	exit(3);
}

/*
Init do aplicativo.
*/
int app_system_init(void)
{
	struct sigaction new_action;
	char s[64];
	int fd;
	int n;

	/* change working directory */
	n = chdir("/tmp");

	/* clear our file mode creation mask */
	umask(0);

	/* Set the current app_pid variable and file */
	app_pid = getpid();

	DBG(DBG_TRACE, "PID: %d", app_pid);

	if ((fd = open(app_pid_path, O_WRONLY | O_CREAT, 
				   S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH)) < 0) {
		DBG(DBG_WARNING, "open(%s)", app_pid_path);
		return -2;
	}

	n = sprintf(s, "%d", app_pid);
	if (write(fd, s, n) <= 0) {
    	close(fd);
        return -3;
    }
	close(fd);

	/* Register the signal handlers */
	/* Set up the structure to specify the new action. */
	new_action.sa_handler = app_termination_handler;
	sigemptyset(&new_action.sa_mask);
	new_action.sa_flags = 0;

	sigaction (SIGINT, &new_action, NULL);
	sigaction (SIGTERM, &new_action, NULL);
	sigaction (SIGQUIT, &new_action, NULL);
	sigaction (SIGABRT, &new_action, NULL);
	sigaction (SIGHUP, &new_action, NULL);

	return 0;
}


/*
Carregar as configurações salvas em arquivo.
*/
int app_system_load_config(void)
{
	DBG(DBG_TRACE, "app_conf_path(\"%s\")", app_conf_path);

	/*
	load values from file
	*/
	conf_load(app_conf_path, conf_root);

	/*
	check or format values
	*/


	/*
	update values to file
	*/
	conf_save(app_conf_path, conf_root);

	return 0;
}

/*
Salvar as configurações do sistema em arquivo.
*/
void app_system_save_config(void)
{
	conf_save(app_conf_path, conf_root);
}

/*
Show usage.
*/
static void app_show_usage(void)
{
	fprintf(stderr, "Usage: %s [OPTION...]\n", app_name);
	fprintf(stderr, "  -h  \tShow this help message\n");
	fprintf(stderr, "  -v  \tShow version\n");
	fprintf(stderr, "  -n  \tprevent the process from running as a daemon\n");
	fprintf(stderr, "\n");
}

/*
Show version.
*/
static void app_show_version(void)
{
	fprintf(stderr, "%s %d.%d (built on " __DATE__ " " __TIME__ ")\n", APP_NAME, APP_VERSION_MAJOR, APP_VERSION_MINOR);
}

/*
Show invalid option.
*/
static void app_parse_err(char * opt)
{
	fprintf(stderr, "%s: invalid option \"%s\"\n", app_name, opt);
}


/*
Main
*/
int main(int argc, char ** argv)
{
	extern char *optarg; /* getopt */
	extern int optind;	 /* getopt */
	(void) optind; //get rid of unnused warning
	int c;

	/* the application name start just after the last slash */
	if ((app_name = (char *)strrchr(argv[0], '/')) == NULL)
		app_name = argv[0];
	else
		app_name++;
	
	/* parse the command line options */
	while ((c = getopt(argc, argv, "vhdn")) > 0) {
		switch (c) {
		case 'v':
			app_show_version();
			return 0;
		case 'h':
			app_show_usage();
			return 1;
		case 'd':
			app_log_enabled++;
			break;
		case 'n':
			/* avoid running in background */
			app_nodaemon++;
			break;
		default:
			app_parse_err(optarg);
			return 2;
		}
	}

	if (app_nodaemon == 0) {
		DBG(DBG_INFO, "Running in background...");
        #if 0
		int pid;

		if ((pid = fork()) < 0) {
			fprintf(stderr, "Can't fork...\n");
			return 2;
		}

		if (pid != 0) {
			/* parent goes bye-bye */
			exit(EXIT_SUCCESS);
		}

		/* child continues */
		/* becomes session leader */
		setsid();
        #else
		//daemon(1, 1);
        #endif	
	}

	DBG(DBG_INFO, "system_init()...");
	if (app_system_init() < 0) {
		DBG(DBG_WARNING, "system_init() fail!");
		return 3;
	}

	DBG(DBG_INFO, "system_config()...");
	if (app_system_load_config() < 0) {
		DBG(DBG_WARNING, "system_load_config() fail!");
		return 4;
	}
	DBG(DBG_INFO, "ctrl_start()...");
	if (ctrl_start(app_fifo_name) < 0) {
		DBG(DBG_WARNING, "ctrl_start(%s) fail!", app_fifo_name);
		app_cleanup();
		return 5;
	}


    DBG(DBG_INFO, "init_monster_shield()...");
    if (init_monster_shield() < 0) {
        DBG(DBG_WARNING, "init_monster_shield() fail!");
        app_cleanup();
        return 6;
    }

    DBG(DBG_INFO, "init_relay()...");
    if (init_relay() < 0) {
        DBG(DBG_WARNING, "init_relay() fail!");
        app_cleanup();
        return 7;
    }

    DBG(DBG_INFO, "servo_init()...");
    if (servo_init() < 0) {
        DBG(DBG_WARNING, "servo_init() fail!");
        app_cleanup();
        return 8;
    } 

    DBG(DBG_INFO, "http_start()...");
	if (http_start() < 0) {
		DBG(DBG_WARNING, "http_start() fail!");
		app_cleanup();
		return 9;
	}

    DBG(DBG_INFO, "server_start()...");
	if (server_start() < 0) {
		DBG(DBG_WARNING, "server_start() fail!");
		app_cleanup();
		return 10;
	}

	DBG(DBG_INFO, "tasks_start()...");
	if (tasks_start() < 0) {
		DBG(DBG_WARNING, "tasks_start() fail!");
		app_cleanup();
		return 11;
	}
    
	if (app_nodaemon) {
		printf("\n");
		printf("----------------------------------------------------------\n");
		printf(" %s %d.%d (built on " __DATE__ " " __TIME__ ")\n", APP_NAME, APP_VERSION_MAJOR, APP_VERSION_MINOR);
		printf("----------------------------------------------------------\n");
		printf("\n");
		fflush(stdout);

		return shell();
	}

	for (;;) {
		sleep(1);
		DBG(DBG_TRACE, "tick");
		sleep(1);
		DBG(DBG_TRACE, "tack");
	}
}

