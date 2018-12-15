/* 
 * File:	tasks.h
 * Author:   Victor Santos (viic.santos@gmail.com)
 * Comment:
 *
 */

#ifndef __TASKS_H__
#define __TASKS_H__

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    COLLECTOR_RUN,
    COLLECTOR_STOP,
    COLLECTOR_TIMER
};

enum {
    AUTO_ON,
    AUTO_OFF
};

enum {
    MOVE_FW,
    MOVE_BW,
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_STOP
  };


int taskOpenCV_run(void);

int taskOpenCV_stop(void);

int taskCollector_run(void);

int taskCollector_stop(void);

int taskCollector_timer(int t);

int taskAuto_run(void);

int taskAuto_stop(void);

int tasks_start(void);

int tasks_stop(void);

#ifdef  __cplusplus
}
#endif

#endif /* __TASKS_H__ */

