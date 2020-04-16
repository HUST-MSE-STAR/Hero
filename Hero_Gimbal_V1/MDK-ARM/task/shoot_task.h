#ifndef SHOOT_TASK_H
#define SHOOT_TASK_H

#include "mytype.h"
#include "pid.h"

#define	FricSpeed1			6500 //ָ������ٶ�Ϊ6500
#define	FricSpeed2			-6500
#define	FricSpeed3			6500 
#define TurnSpeed				-4000
#define BlockSpeed			7000
#define BlockCurrent		3000.0f

#define turn_angle_delta  6*8194.00f //���̽Ƕ�6�ȷ�,�о�������Ǹ����ٱ��й�


typedef struct shoot *shoot_t;

struct shoot			//����ṹ��
{
	int16_t fric1;
	int16_t fric2;
	int16_t fric3;
	
	pid_struct_t position_pid;
	float turn_angle;
	float speed_target;
  float angle_target;

};

extern pid_struct_t shoot_pid[3];

void shoot_task(void const * argument);
void shoot_init(shoot_t pshoot);
void shoot_control(shoot_t pshoot);


void shoot_set_turn_angle_delta(float turn_angle);
static void shoot_block_check(shoot_t pshoot);
#endif

