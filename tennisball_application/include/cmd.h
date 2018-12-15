/* 
 * File:	cmd.h
 * Author:   Victor Santos (viic.santos@gmail.com)
 * Comment:
 * 
 */

#ifndef __CMD_H__
#define __CMD_H__

#ifdef __cplusplus
extern "C" {
#endif

int cmd_line_parse(char * s, char ** argv, int argmax);

int cmd_exec(int argc, char * argv[]);

#ifdef  __cplusplus
}
#endif

#endif /* __CMD_H__ */

