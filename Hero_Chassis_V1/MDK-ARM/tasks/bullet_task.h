#ifndef Bullet_Task_H
#define Bullet_Task_H
#include "cmsis_os.h"
#include "pid.h"
void Bullet_Task(void const * argument);
struct shoot			//射击结构体
{
	int16_t fric1;
	int16_t fric2;
	int16_t fric3;
	
	pid_struct_t position_pid;
	float turn_angle;
	float speed_target;
  float angle_target;

};
typedef struct shoot *shoot_t;
uint32_t get_time_ms(void);
void bullet_block_check(shoot_t pshoot);
void bullet_control(shoot_t pshoot) ;
#define TurnSpeed				-4000
#define BlockSpeed			7000
#define BlockCurrent		3000.0f

#define turn_angle_delta  6*8194.00f  //到时候还得改
#endif
