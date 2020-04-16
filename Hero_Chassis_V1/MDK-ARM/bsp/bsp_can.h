
#ifndef __BSP_CAN
#define __BSP_CAN

#include "can.h"

#define FEEDBACK_ID_BASE_C620      0x201
#define FEEDBACK_ID_BASE_6020      0x205
#define GIMBAL_ID 100
#define CAN_CONTROL_ID_BASE   0x1ff
#define CAN_CONTROL_ID_EXTEND 0x2ff
#define MOTOR_MAX_NUM         4

#define M3508M2006_TXID 1
#define M6020_TXID 0

typedef struct
{
		uint8_t  initFlag;
	  int16_t  set_voltage;
    uint16_t can_id;
    uint16_t rotor_angle;
    int16_t  rotor_speed;
    int16_t  torque_current;
    uint8_t  temp;
		int16_t  round_cnt;
	  int32_t  all_angle;
}moto_info_t;

extern moto_info_t motor_info[5];
extern moto_info_t last_motor_info[5];

extern _Bool id_range;

void can_user_init(CAN_HandleTypeDef* hcan);
void set_motor_voltage(_Bool id_range,int16_t v1, int16_t v2, int16_t v3, int16_t v4);
void Bullet_Task(void const * argument);//²¦µ¯ÈÎÎñ
#endif
















