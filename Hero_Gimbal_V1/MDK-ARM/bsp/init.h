#ifndef INIT_H
#define INIT_H

#include "main.h"
#include "cmsis_os.h"

extern osThreadId gimbal_taskhandle;
extern osThreadId communication_to_chasis_taskhandle;
extern osThreadId shoot_taskhandle;
extern osThreadId communication_to_pc_taskhandle;
 
void hw_init(void);
void task_init(void);


#endif

