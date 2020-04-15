#include "bsp_can.h"
#include "pid.h"
#include "bsp_dbus.h"
#include "shoot_task.h"
#include "cmsis_os.h"
#include "main.h"
#include <stdio.h>

struct shoot sshoot; //����shoot�ṹ��
shoot_t pshoot = &sshoot;
int32_t shoot_cnt = 0,first_angle=0;
pid_struct_t shoot_pid[3];
int8_t shoot_begin=0,shoot_once=0,shoot_continue=0;
uint32_t shoot_mode_check=0;
void shoot_init(shoot_t pshoot)//����������ٶȻ�
{
	pid_init(&(shoot_pid[0]),    	10,  0, 0,	3000, 	10000);  //Ħ����
	pid_init(&(shoot_pid[1]),  		10, 	0,0, 	3000, 	10000);  //Ħ����
	pid_init(&(shoot_pid[2]),  		10, 	0,0, 	3000, 	10000);  //Ħ����
//	pid_init(&(pshoot->position_pid), 0.05, 	0, 	   0, 	3000, 	10000);
	
	pshoot->fric1=0;
	pshoot->fric2=0;
	pshoot->fric3=0;
	pshoot->speed_target=0;
	pshoot->angle_target=0;
	pshoot->turn_angle=turn_angle_delta;

}

void shoot_task(void const * argument)
{
	uint32_t period = osKernelSysTick();
	shoot_init(pshoot);
	while(1)
	{
		if(rc_device_get_state(&rc,RC_S2_UP))//ң�������ƻ������˶�
		{
			if(rc_device_get_state(&rc, RC_S1_MID2UP))//���S1���д���
			{
				pshoot->fric1=FricSpeed1; //����Ħ����ת��
				pshoot->fric2=FricSpeed2;
				pshoot->fric3=FricSpeed3;		
				shoot_begin=1;        //��ʼ���
				
			}

		}
//		printf("%d\r\n",motor_info[2].torque_current);
	shoot_control(pshoot);
	osDelayUntil(&period, 5);
	}
}

void shoot_control(shoot_t pshoot) 
{
	int id_range=1;//3508��2006�ĵ��
 	motor_info[0].set_voltage=pid_calc(&shoot_pid[0],	pshoot->fric1,motor_info[0].rotor_speed);	
	motor_info[1].set_voltage=pid_calc(&shoot_pid[1],	pshoot->fric2,motor_info[1].rotor_speed);

	
	motor_info[2].set_voltage=pid_calc(&shoot_pid[2],	pshoot->fric3,motor_info[2].rotor_speed);
  set_motor_voltage(id_range,motor_info[0].set_voltage,motor_info[1].set_voltage,motor_info[2].set_voltage,0);
		
}




