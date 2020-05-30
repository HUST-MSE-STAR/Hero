/* 
 * bsp_can.c-can通信相关文件
 * NOTE: This file is based on HAL library of stm32 platform
 *       包括can初始化,can接收和can发送
 * Copyright (c) 2020-, FOSH Project
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:None
 */
#include "bsp_can.h"

moto_info_t motor_info[5];
moto_info_t last_motor_info[5];
_Bool id_range;

/**
    * @brief  用于can通信初始化
    * @note   None
    * @author 周森
    * @param  None
    * @retval None
    */
void can_user_init(CAN_HandleTypeDef* hcan )
{
  CAN_FilterTypeDef  can_filter;

  can_filter.FilterBank = 0;          /*筛选器编号，范围 0-27*/           
  can_filter.FilterMode =  CAN_FILTERMODE_IDMASK;   /*筛选器模式 */
  can_filter.FilterScale = CAN_FILTERSCALE_32BIT;   /*设置筛选器的尺度 */ 
  can_filter.FilterIdHigh = 0;          /*CAN_FxR1 寄存器的高 16 位 */
  can_filter.FilterIdLow  = 0;           /*CAN_FxR1 寄存器的低 16 位*/
  can_filter.FilterMaskIdHigh = 0;      /*CAN_FxR2 寄存器的高 16 位*/
  can_filter.FilterMaskIdLow  = 0;       /*CAN_FxR2 寄存器的低 16 位 */         
  can_filter.FilterFIFOAssignment = CAN_RX_FIFO0;     /*设置经过筛选后数据存储到哪个接收 FIFO */ 
  can_filter.FilterActivation = ENABLE;       /*是否使能本筛选器*/    
           
  HAL_CAN_ConfigFilter(hcan, &can_filter);        //初始化筛选器
 // HAL_CAN_Start(&hcan1);                          // start can1
  HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING); // enable can1 rx interrupt
}

/**
    * @brief  用于can接收中断
    * @note   None
    * @author 周森V(1),钟午杰V(2)
    * @param  hcan
    * @retval None
    */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  CAN_RxHeaderTypeDef rx_header;
  uint8_t             rx_data[8];
  if(hcan->Instance == CAN)
  {
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data); //receive can data
  }
  if ((rx_header.StdId >= FEEDBACK_ID_BASE_C620)
   && (rx_header.StdId <  FEEDBACK_ID_BASE_C620 + MOTOR_MAX_NUM))
	{

    uint8_t index = rx_header.StdId - FEEDBACK_ID_BASE_C620;                  // get motor index by can_id
		last_motor_info[index] = motor_info[index];
    motor_info[index].rotor_angle    = ((rx_data[0] << 8) | rx_data[1]);
    motor_info[index].rotor_speed    = ((rx_data[2] << 8) | rx_data[3]);
    motor_info[index].torque_current = ((rx_data[4] << 8) | rx_data[5]);
    motor_info[index].temp           =   rx_data[6];
		motor_info[index].round_cnt      = last_motor_info[index].round_cnt;
		motor_info[index].all_angle=motor_info[index].rotor_angle+8191 * motor_info[index].round_cnt;//all_angle为电机转过的总角度,1圈为8191
		
		if(motor_info[index].rotor_angle - last_motor_info[index].rotor_angle > 4096)
			motor_info[index].round_cnt --;
		else if(motor_info[index].rotor_angle - last_motor_info[index].rotor_angle < -4096)
			motor_info[index].round_cnt ++;		
	}
  else if ((rx_header.StdId >= FEEDBACK_ID_BASE_6020)
   && (rx_header.StdId <  FEEDBACK_ID_BASE_6020 + MOTOR_MAX_NUM))                  // judge the can id
  {
    uint8_t index = rx_header.StdId - FEEDBACK_ID_BASE_6020+4;                  // get motor index by can_id
		last_motor_info[index] = motor_info[index];
    motor_info[index].rotor_angle    = ((rx_data[0] << 8) | rx_data[1]);
    motor_info[index].rotor_speed    = ((rx_data[2] << 8) | rx_data[3]);
    motor_info[index].torque_current = ((rx_data[4] << 8) | rx_data[5]);
    motor_info[index].temp           =   rx_data[6];
		motor_info[index].round_cnt      = last_motor_info[index].round_cnt;
		motor_info[index].all_angle=motor_info[index].rotor_angle+8191 * motor_info[index].round_cnt;
		
		if(motor_info[index].initFlag == 1)
		{
			if(motor_info[index].rotor_angle - last_motor_info[index].rotor_angle > 4096)
				motor_info[index].round_cnt --;
			else if(motor_info[index].rotor_angle - last_motor_info[index].rotor_angle < -4096)
				motor_info[index].round_cnt ++;
		}
		else
		{
			motor_info[index].initFlag = 1;
		}
  }

}

/**
    * @brief  用于设置电机的电流
    * @note   id_range用于区分是前4个(0x200)还是后4个电机(0x1ff)
    * @author 周森V(1),钟午杰V(2)
    * @param  id_range(0/1),四个电机编号:v1,v2,v3,v4
    * @retval None
    */
void set_motor_voltage(_Bool id_range,int16_t v1, int16_t v2, int16_t v3, int16_t v4)
{
  CAN_TxHeaderTypeDef tx_header;
  uint8_t             tx_data[8];

	tx_header.StdId = (id_range == 0)?(0x1ff):(0x200);//6020:0,0x1ff  3508/2006:1,0x200
	tx_header.IDE   = CAN_ID_STD;  //区分标准格式与扩展格
  tx_header.RTR   = CAN_RTR_DATA;  //区分数据帧和遥控帧的
  tx_header.DLC   = 8;     //数据段含有多少个字节

  tx_data[0] = (v1>>8)&0xff;
  tx_data[1] =    (v1)&0xff;
  tx_data[2] = (v2>>8)&0xff;
  tx_data[3] =    (v2)&0xff;
  tx_data[4] = (v3>>8)&0xff;
  tx_data[5] =    (v3)&0xff;
  tx_data[6] = (v4>>8)&0xff;
  tx_data[7] =    (v4)&0xff;
  HAL_CAN_AddTxMessage(&hcan, &tx_header, tx_data,(uint32_t*)CAN_TX_MAILBOX0); 
}







