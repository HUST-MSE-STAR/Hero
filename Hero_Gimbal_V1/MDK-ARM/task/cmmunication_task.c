/* 
 * chassis_task.c-通信相关文件
 * NOTE: 包括串口通信解析以及与底盘控制板的通信
 *
 * Copyright (c) 2020-, FOSH Project
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * C*/
#include "string.h"
#include "stdlib.h"
#include "communication_task.h"
#include "bsp_dbus.h"
#include "init.h"
#include "cmsis_os.h"
#include "bsp_can.h"
#include <stdio.h>
#include "gimbal_task.h"
#include "bsp_referee.h"
#include "bsp_can.h"
#include "bsp_dbus.h"
extern uint8_t 	PC_buf[PC_BUFLEN] ;

/**
    * @brief  将yaw,pitch角度存入pgimbal结构体中
    * @note   None
    * @author 钟午杰
    * @param  None
    * @retval None
    */
void calc_rev_buff(void )//云台的接收函数
{
	
		pgimbal->PC_target_angle.yaw=((PC_buf[2]<<8)||PC_buf[3]);
    pgimbal->PC_target_angle.pitch=((PC_buf[4]<<8)||PC_buf[5]);
	
}

/**
    * @brief  用于和PC的通信
    * @note   None
    * @author 钟午杰
    * @param  None
    * @retval None
    */
void Communication_To_Pc_Task(void const * argument)//把裁判系统的信息传给PC
{
	uint32_t period = osKernelSysTick();
	while(1)
	{
		osSignalWait(REFEREE_UPDATA, osWaitForever);
		HAL_UART_Transmit_DMA(&PC_HUART, (uint8_t *)REFEREE_buf, sizeof(REFEREE_buf)); 
		osDelayUntil(&period, 10);
	}
}

/**
    * @brief  用于和底盘的通信
    * @note   None
    * @author 钟午杰
    * @param  None
    * @retval None
    */
void Communication_To_Chasis_Task(void const * argument)//把遥控信息传给底盘
{
	uint32_t period = osKernelSysTick();
	while(1)
	{
		osSignalWait(RC_UPDATA, osWaitForever);
		CAN_TxHeaderTypeDef tx_header;
    tx_header.StdId = 100;//云台ID为100
	  tx_header.IDE   = CAN_ID_STD;  //区分标准格式与扩展格
    tx_header.RTR   = CAN_RTR_DATA;  //区分数据帧和遥控帧的
    tx_header.DLC   = sizeof(dbus_buf);     //数据段含有多少个字节
    HAL_CAN_AddTxMessage(&hcan2, &tx_header, dbus_buf,(uint32_t*)CAN_TX_MAILBOX0); 
		osDelayUntil(&period, 10);
	}
}




