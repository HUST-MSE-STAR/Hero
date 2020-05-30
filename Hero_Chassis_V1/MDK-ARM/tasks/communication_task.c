/* 
 * chassis_task.c-通信相关文件
 * NOTE: 包括串口通信解析以及与云台控制板的通信
 *
 * Copyright (c) 2020-, FOSH Project
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * C*/
#include "communication_task.h"
#include "bsp_dbus.h"
#include "init.h"
#include "cmsis_os.h"
#include <stdio.h>
#include "bsp_referee.h"
#include "bsp_can.h"

Gimbal_State gimbal_state;
uint8_t   x=1;

/**
    * @brief  用于裁判系统的通信解析
    * @note   None
    * @author 钟午杰
    * @param  None
    * @retval None
    */
void Communication_To_Referee_Task(void const * argument)//与裁判系统进行通信
{	
	uint32_t period = osKernelSysTick();
	

	while(1)
	{
		osSignalWait(RC_UPDATA, osWaitForever); //等信号过来就进行裁判系统数据解压
		referee_unpack_fifo_data();
		osDelayUntil(&period, 2); //这个任务将在period+2ms解除阻塞
	}
}

/**
    * @brief  用于和云台的通信
    * @note   None
    * @author 钟午杰
    * @param  None
    * @retval None
    */
void Communication_To_Gimbal_Task(void const * argument)//与云台通信
{
	uint32_t period = osKernelSysTick();
	while(1)
	{
		osSignalWait(REFEREE_UPDATA, osWaitForever);
		CAN_TxHeaderTypeDef tx_header;
    tx_header.StdId = 110;//底盘ID设为110
	  tx_header.IDE   = CAN_ID_STD;  //区分标准格式与扩展格
    tx_header.RTR   = CAN_RTR_DATA;  //区分数据帧和遥控帧的
    tx_header.DLC   = sizeof(referee_data_buff.protocol_packet);     //数据段含有多少个字节
    HAL_CAN_AddTxMessage(&hcan, &tx_header, referee_data_buff.protocol_packet,(uint32_t*)CAN_TX_MAILBOX0); 
		osDelayUntil(&period, 5); //这个任务将在period+2ms解除阻塞
	}
}
