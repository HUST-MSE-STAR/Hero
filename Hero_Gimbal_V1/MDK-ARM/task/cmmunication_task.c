/* 
 * chassis_task.c-ͨ������ļ�
 * NOTE: ��������ͨ�Ž����Լ�����̿��ư��ͨ��
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
    * @brief  ��yaw,pitch�Ƕȴ���pgimbal�ṹ����
    * @note   None
    * @author �����
    * @param  None
    * @retval None
    */
void calc_rev_buff(void )//��̨�Ľ��պ���
{
	
		pgimbal->PC_target_angle.yaw=((PC_buf[2]<<8)||PC_buf[3]);
    pgimbal->PC_target_angle.pitch=((PC_buf[4]<<8)||PC_buf[5]);
	
}

/**
    * @brief  ���ں�PC��ͨ��
    * @note   None
    * @author �����
    * @param  None
    * @retval None
    */
void Communication_To_Pc_Task(void const * argument)//�Ѳ���ϵͳ����Ϣ����PC
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
    * @brief  ���ں͵��̵�ͨ��
    * @note   None
    * @author �����
    * @param  None
    * @retval None
    */
void Communication_To_Chasis_Task(void const * argument)//��ң����Ϣ��������
{
	uint32_t period = osKernelSysTick();
	while(1)
	{
		osSignalWait(RC_UPDATA, osWaitForever);
		CAN_TxHeaderTypeDef tx_header;
    tx_header.StdId = 100;//��̨IDΪ100
	  tx_header.IDE   = CAN_ID_STD;  //���ֱ�׼��ʽ����չ��
    tx_header.RTR   = CAN_RTR_DATA;  //��������֡��ң��֡��
    tx_header.DLC   = sizeof(dbus_buf);     //���ݶκ��ж��ٸ��ֽ�
    HAL_CAN_AddTxMessage(&hcan2, &tx_header, dbus_buf,(uint32_t*)CAN_TX_MAILBOX0); 
		osDelayUntil(&period, 10);
	}
}




