#ifndef __COMMUNICATION_H
#define __COMMUNICATION_H

#include "usart.h"
#include "bsp_dbus.h"




typedef struct
{
	float abs_yaw_17;
	float abs_pitch_17;
	int16_t rel_yaw_17;
	int16_t rel_pitch_17;
}Gimbal_State;

void calc_rev_buff(uint8_t* buff,uint8_t len,uint8_t dir);
void Communication_To_Referee_Task(void const * argument);
void Communication_To_Gimbal_Task(void const * argument);
extern Gimbal_State gimbal_state;

#endif

