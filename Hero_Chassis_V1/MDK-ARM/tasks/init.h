#ifndef INIT_H
#define INIT_H

#include "main.h"
#include "cmsis_os.h"


extern osThreadId chassis_taskhandle;
extern osThreadId CommunicationToReferee_taskhandle;
extern osThreadId CommunicationToGimbal_taskhandle;

void hw_init(void);
void task_init(void);


#endif















