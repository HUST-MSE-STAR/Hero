#include "bsp_can.h"
#include "pid.h"
#include "bsp_dbus.h"
#include "init.h"
#include "gimbal_task.h"
#include "communication_task.h"
#include "shoot_task.h"

void hw_init(void)
{

	HAL_CAN_Start(&hcan1);                          // start can1
  can_user_init(&hcan1);
	HAL_CAN_Start(&hcan2);                          // start can2
  can_user_init(&hcan2);
	dbus_uart_init();
}

osThreadId gimbal_taskhandle;
osThreadId communication_to_pc_taskhandle;
osThreadId communication_to_chasis_taskhandle;
osThreadId shoot_taskhandle;



void task_init(void)
{
	osThreadDef(GIMBAL_TASK, gimbal_task, osPriorityNormal, 0, 512);
  gimbal_taskhandle = osThreadCreate(osThread(GIMBAL_TASK), NULL);
	
	osThreadDef(COMMUNICATION_TO_PC_TASK, Communication_To_Pc_Task, osPriorityNormal, 0, 512);
  communication_to_pc_taskhandle = osThreadCreate(osThread(COMMUNICATION_TO_PC_TASK), NULL);
		
	osThreadDef(SHOOT_TASK, shoot_task, osPriorityNormal, 0, 512);
  shoot_taskhandle = osThreadCreate(osThread(SHOOT_TASK), NULL);	
	
	osThreadDef(COMMUNICATION_TO_CHASIS_TASK, Communication_To_Chasis_Task, osPriorityNormal, 0, 512);
  communication_to_chasis_taskhandle = osThreadCreate(osThread(COMMUNICATION_TO_CHASIS_TASK), NULL);
	
}

