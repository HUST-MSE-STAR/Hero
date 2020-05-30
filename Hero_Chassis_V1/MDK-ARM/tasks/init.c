/* 
 * init.c-初始化文件
 * NOTE: This file is based on HAL library of stm32 platform
 *
 * Copyright (c) 2020-, FOSH Project
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:None
 */
#include "bsp_can.h"
#include "pid.h"
#include "bsp_dbus.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "chassis_task.h"
#include "communication_task.h"
#include "init.h"

/*用于系统初始化*/
void hw_init(void)
{

	HAL_CAN_Start(&hcan);                          // start can1
  can_user_init(&hcan);
	dbus_uart_init();

}

osThreadId chassis_taskhandle;
osThreadId CommunicationToReferee_taskhandle;
osThreadId CommunicationToGimbal_taskhandle;

/*用于初始化任务*/
void task_init(void)//现在分配的空间只够创建6个任务
{
  osThreadDef(CHASSIS_TASK, chassis_task, osPriorityNormal, 0, 512);
  chassis_taskhandle = osThreadCreate(osThread(CHASSIS_TASK), NULL);
  osThreadDef(COMMUNICATION_TO_REfEREE_TASK, Communication_To_Referee_Task, osPriorityNormal, 0, 512);//和裁判系统通信
  CommunicationToReferee_taskhandle = osThreadCreate(osThread(COMMUNICATION_TO_REfEREE_TASK), NULL);	
	osThreadDef(COMMUNICATION_TO_GIMBAL_TASK, Communication_To_Gimbal_Task, osPriorityNormal, 0, 512);  
  CommunicationToGimbal_taskhandle = osThreadCreate(osThread(COMMUNICATION_TO_GIMBAL_TASK), NULL);	
	osThreadDef(BULLET_TASK, Bullet_Task, osPriorityNormal, 0, 512);
  chassis_taskhandle = osThreadCreate(osThread(BULLET_TASK), NULL);
}


