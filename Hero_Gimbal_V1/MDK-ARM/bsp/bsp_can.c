#include "bsp_can.h"
#include <string.h>
#include "bsp_dbus.h"
#include  "cmsis_os.h"
#include "init.h"
#include "bsp_referee.h"
moto_info_t motor_info[6];//��ʵֻ��5�����
moto_info_t last_motor_info[6];
_Bool id_range;
uint8_t        protocol_packet[REF_PROTOCOL_FRAME_MAX_SIZE];//����ϵͳ������
void can_user_init(CAN_HandleTypeDef* hcan )
{
  CAN_FilterTypeDef  can_filter;

  can_filter.FilterBank = 0;          /*ɸѡ����ţ���Χ 0-27*/           
  can_filter.FilterMode =  CAN_FILTERMODE_IDMASK;   /*ɸѡ��ģʽ */
  can_filter.FilterScale = CAN_FILTERSCALE_32BIT;   /*����ɸѡ���ĳ߶� */ 
  can_filter.FilterIdHigh = 0;          /*CAN_FxR1 �Ĵ����ĸ� 16 λ */
  can_filter.FilterIdLow  = 0;           /*CAN_FxR1 �Ĵ����ĵ� 16 λ*/
  can_filter.FilterMaskIdHigh = 0;      /*CAN_FxR2 �Ĵ����ĸ� 16 λ*/
  can_filter.FilterMaskIdLow  = 0;       /*CAN_FxR2 �Ĵ����ĵ� 16 λ */         
  can_filter.FilterFIFOAssignment = CAN_RX_FIFO0;     /*���þ���ɸѡ�����ݴ洢���ĸ����� FIFO */ 
  can_filter.FilterActivation = ENABLE;       /*�Ƿ�ʹ�ܱ�ɸѡ��*/    
           
  HAL_CAN_ConfigFilter(hcan, &can_filter);        //��ʼ��ɸѡ��
 // HAL_CAN_Start(&hcan1);                          // start can1
  HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING); // enable can rx interrupt
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  CAN_RxHeaderTypeDef rx_header;
  uint8_t             rx_data[8];
  if(hcan->Instance == CAN2)
  {
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data); //receive can data
  
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
		motor_info[index].all_angle=motor_info[index].rotor_angle+8191 * motor_info[index].round_cnt;//all_angleΪ���ת�����ܽǶ�,1ȦΪ8191
		
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
	else if(hcan->Instance == CAN1)//�͵��̵�ͨ��
  {
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data); //receive can data
		if(rx_header.StdId==110) strcpy((char *)REFEREE_buf,(const char *)rx_data); //����CAN��ID��110
		osSignalSet(communication_to_pc_taskhandle, REFEREE_UPDATA);	
	}
}

void set_motor_voltage(_Bool id_range,int16_t v1, int16_t v2, int16_t v3, int16_t v4)
{
  CAN_TxHeaderTypeDef tx_header;
  uint8_t             tx_data[8];

	tx_header.StdId = (id_range == 0)?(0x1ff):(0x200);//6020:0,0x1ff  3508/2006:1,0x200
	tx_header.IDE   = CAN_ID_STD;  //���ֱ�׼��ʽ����չ��
  tx_header.RTR   = CAN_RTR_DATA;  //��������֡��ң��֡��
  tx_header.DLC   = 8;     //���ݶκ��ж��ٸ��ֽ�

  tx_data[0] = (v1>>8)&0xff;
  tx_data[1] =    (v1)&0xff;
  tx_data[2] = (v2>>8)&0xff;
  tx_data[3] =    (v2)&0xff;
  tx_data[4] = (v3>>8)&0xff;
  tx_data[5] =    (v3)&0xff;
  tx_data[6] = (v4>>8)&0xff;
  tx_data[7] =    (v4)&0xff;
  HAL_CAN_AddTxMessage(&hcan1, &tx_header, tx_data,(uint32_t*)CAN_TX_MAILBOX0); 
}






