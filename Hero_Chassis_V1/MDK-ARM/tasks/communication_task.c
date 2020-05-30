/* 
 * chassis_task.c-ͨ������ļ�
 * NOTE: ��������ͨ�Ž����Լ�����̨���ư��ͨ��
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
    * @brief  ���ڲ���ϵͳ��ͨ�Ž���
    * @note   None
    * @author �����
    * @param  None
    * @retval None
    */
void Communication_To_Referee_Task(void const * argument)//�����ϵͳ����ͨ��
{	
	uint32_t period = osKernelSysTick();
	

	while(1)
	{
		osSignalWait(RC_UPDATA, osWaitForever); //���źŹ����ͽ��в���ϵͳ���ݽ�ѹ
		referee_unpack_fifo_data();
		osDelayUntil(&period, 2); //���������period+2ms�������
	}
}

/**
    * @brief  ���ں���̨��ͨ��
    * @note   None
    * @author �����
    * @param  None
    * @retval None
    */
void Communication_To_Gimbal_Task(void const * argument)//����̨ͨ��
{
	uint32_t period = osKernelSysTick();
	while(1)
	{
		osSignalWait(REFEREE_UPDATA, osWaitForever);
		CAN_TxHeaderTypeDef tx_header;
    tx_header.StdId = 110;//����ID��Ϊ110
	  tx_header.IDE   = CAN_ID_STD;  //���ֱ�׼��ʽ����չ��
    tx_header.RTR   = CAN_RTR_DATA;  //��������֡��ң��֡��
    tx_header.DLC   = sizeof(referee_data_buff.protocol_packet);     //���ݶκ��ж��ٸ��ֽ�
    HAL_CAN_AddTxMessage(&hcan, &tx_header, referee_data_buff.protocol_packet,(uint32_t*)CAN_TX_MAILBOX0); 
		osDelayUntil(&period, 5); //���������period+2ms�������
	}
}
