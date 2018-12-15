/*
 * File:	application.h
 * Author:   Victor Santos (viic.santos@gmail.com)
 * Comment:
 *
 */

#ifndef __APP_H__
#define __APP_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
Application properties
*/
#define APP_NAME "application"
#define APP_VERSION_MAJOR 0
#define APP_VERSION_MINOR 1


/*
public vars
*/


/*
Application
*/
extern int app_pid;                 // PID do main

extern int app_nodaemon;            // 0 = rodando em background

extern int app_null_fd;             // file descriptor para /dev/null

extern char app_null[];             // null char para usar com /dev/null


/*
prototypes
*/


/*
Retorna ponteiro para o inicio do nome do arquivo.
*/
extern char * app_extract_fname(char * fname);

/*
Salvar as configurações no arquivo.
*/
extern void app_system_save_config(void);



#ifdef  __cplusplus
}
#endif

#endif /* __APP_H__ */

