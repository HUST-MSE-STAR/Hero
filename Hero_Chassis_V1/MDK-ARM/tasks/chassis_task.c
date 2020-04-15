#include "chassis_task.h"
#include "bsp_can.h"
#include "pid.h"
#include "bsp_dbus.h"
#include <stdio.h>
#include "math.h"

struct chassis schassis;
chassis_t pchassis = &schassis;
pid_struct_t chassis_motor_pid[4];
float vx, vy, wz;		//�����ٶ�
float location_vx,location_vy;//���ԭ����ϵ���ٶ�

void chassis_init(void)//��ʼ�� �ٶ� ���ٶ� λ�� ����PID
{
	
	pchassis->ax_mm_s2=0;
	pchassis->ay_mm_s2=0;
	pchassis->az_deg_s2=0;
	
	pchassis->xv_mm_s=0;
	pchassis->yv_mm_s=0;
	pchassis->zw_deg_s=0;
	
	pchassis->position_x_mm=0;
	pchassis->position_y_mm=0;
	pchassis->angle_deg=0;
		
	pchassis->wheel_perimeter=152*PI; //����������ܳ�
	pchassis->wheeltrack=538;
	pchassis->wheelbase=509;
	
  pid_init(&chassis_motor_pid[0],25,0.05,0,2000,INIT_current);//INIT_currentΪ����������
	pid_init(&chassis_motor_pid[1],25,0.05,0,2000,INIT_current);
	pid_init(&chassis_motor_pid[2],25,0.05,0,2000,INIT_current);
	pid_init(&chassis_motor_pid[3],25,0.05,0,2000,INIT_current);

}	

void chassis_task(void const * argument)
{
	uint32_t period = osKernelSysTick();
	chassis_init();
	dbus_uart_init();
  float degree;
	while(1)
	{		
      if (rc_device_get_state(&rc, RC_S2_UP))				//��sw2ΪUP�����̸�����̨���ٶ���ң����ch1-2���ƣ����ٶ�PID��follow_relative_angle����
      {//�൱����ҡ�˵�����ͨ������x,y
				//(1684-364)/2
        vx = (float)rc.ch2 / 660 *MAX_CHASSIS_VX_SPEED;   //ch2����x�����ٶ�
        vy = (float)rc.ch1 / 660 *MAX_CHASSIS_VX_SPEED;   //ch1����y�����ٶ�
        wz =-(float)rc.wheel/ 660 *MAX_CHASSIS_VW_SPEED;	//follow_relative_angle������gimbal_info_rcv				
      }//���ٶ�Ӧ��������̨�İ��ӿ���
      if (rc_device_get_state(&rc, RC_S2_MID))			//��S2���м�,���ɼ��̿���
      {
				if(rc_device_get_key_press(&rc, KEY_PRESSED_W))
        vx = MAX_CHASSIS_VX_SPEED;
				if(rc_device_get_key_press(&rc, KEY_PRESSED_S))
        vx = -MAX_CHASSIS_VX_SPEED;
				if(rc_device_get_key_press(&rc, KEY_PRESSED_A))				
        vy = MAX_CHASSIS_VY_SPEED; 
				if(rc_device_get_key_press(&rc, KEY_PRESSED_D))
        vy = -MAX_CHASSIS_VY_SPEED;	
				
      }
			if(rc_device_get_state(&rc, RC_S2_DOWN))//S2ΪDOWNʱ������С����ģʽ����Ȼ����ң������
			{
				location_vx = (float)rc.ch2 / 660 *MAX_CHASSIS_VX_SPEED;   //ch2����x�����ٶ�
        location_vy = (float)rc.ch1 / 660 *MAX_CHASSIS_VX_SPEED;   //ch1����y�����ٶ�
        wz =2000;	//�ݶ�ת��Ϊ2000 
				degree=calculate_the_degree();
				vx=location_vx*cos(degree)+location_vy*sin(degree);
				vy=-location_vx*sin(degree)+location_vy*cos(degree);
			}
	chassis_set_speed(vx,vy,wz);
	chassis_execute();
	osDelayUntil(&period, 2);		
	}
}

void chassis_set_speed(float vx_mmps,float vy_mmps,float wz_degps) //����ǰѵõ���xv,yv,zw��������ָ����
{
	pchassis->xv_mm_s=vx_mmps;
	pchassis->yv_mm_s=vy_mmps;
	pchassis->zw_deg_s=wz_degps; 	
}
void chassis_execute()//���̿���
{
	mecanum_calculate();	//�����ٶȷֽ⵽�ĸ�������
		for (int i = 0; i < 4; i++)		//�������ĸ������ķ�����
	{	
		chassis_motor_pid[i].output=pid_calc(&chassis_motor_pid[i],pchassis->wheel_rpm[i],motor_info[i].rotor_speed);		
	}
	set_motor_voltage(M3508M2006_TXID,chassis_motor_pid[0].output,chassis_motor_pid[1].output,chassis_motor_pid[2].output,chassis_motor_pid[3].output);
  //ͨ��PIDת��Ϊ��ѹ���
}

void mecanum_calculate(void)//�����ٶȷֽ⵽�ĸ�������	
{
	static float rotate_ratio_fr;//ǰ����ת�� 
	static float rotate_ratio_fl;//ǰ����ת��
	static float rotate_ratio_bl;//������ת��
	static float rotate_ratio_br;//������ת��
	static float wheel_rpm_ratio;

	rotate_ratio_fr = ((pchassis->wheelbase + pchassis->wheeltrack) / 2.0f - pchassis->rotate_x_offset + pchassis->rotate_y_offset) / 57.3f;
	rotate_ratio_fl = ((pchassis->wheelbase + pchassis->wheeltrack) / 2.0f - pchassis->rotate_x_offset - pchassis->rotate_y_offset) / 57.3f;
	rotate_ratio_bl = ((pchassis->wheelbase + pchassis->wheeltrack) / 2.0f + pchassis->rotate_x_offset - pchassis->rotate_y_offset) / 57.3f;
	rotate_ratio_br = ((pchassis->wheelbase + pchassis->wheeltrack) / 2.0f + pchassis->rotate_x_offset + pchassis->rotate_y_offset) / 57.3f;
  //ǰ������Ӧ���ǳ��ĳ��Ϳ�,��������Ӧ���ǳ�������ƫ��,����57.3���൱�ڰѶȱ�ɻ���
	wheel_rpm_ratio = 60.0f / (pchassis->wheel_perimeter * MOTOR_DECELE_RATIO);//60/(�����ܳ�*���ٱ�)
  //���û�������ת���Ǹ���һ��

	MEC_VAL_LIMIT(pchassis->xv_mm_s, -MAX_CHASSIS_VX_SPEED, MAX_CHASSIS_VX_SPEED); //mm/s //����һ�¸����ٶȵķ�Χ
	MEC_VAL_LIMIT(pchassis->yv_mm_s, -MAX_CHASSIS_VY_SPEED, MAX_CHASSIS_VY_SPEED); //mm/s
	MEC_VAL_LIMIT(pchassis->zw_deg_s, -MAX_CHASSIS_VW_SPEED, MAX_CHASSIS_VW_SPEED); //deg/s

	float wheel_rpm[4];
	float max = 0;
  //mm/s,����/s��Ϊr/min
	wheel_rpm[0] = (pchassis->xv_mm_s - pchassis->yv_mm_s + pchassis->zw_deg_s * rotate_ratio_fr) * wheel_rpm_ratio;//�൱�ڽ��ٶ�*6.63,��������˸�����
	wheel_rpm[1] = (pchassis->xv_mm_s + pchassis->yv_mm_s + pchassis->zw_deg_s * rotate_ratio_fl) * wheel_rpm_ratio;
	wheel_rpm[2] = (-pchassis->xv_mm_s + pchassis->yv_mm_s + pchassis->zw_deg_s * rotate_ratio_bl) * wheel_rpm_ratio;
	wheel_rpm[3] = (-pchassis->xv_mm_s - pchassis->yv_mm_s + pchassis->zw_deg_s * rotate_ratio_br) * wheel_rpm_ratio;
  //����λ��Ϊr/min
	//find max item
	for (uint8_t i = 0; i < 4; i++)
	{
		if (fabs(wheel_rpm[i]) > max)
			max = fabs(wheel_rpm[i]);
	}

	//equal proportion
	if (max > MAX_WHEEL_RPM)
	{
		float rate = MAX_WHEEL_RPM / max;
		for (uint8_t i = 0; i < 4; i++)
			wheel_rpm[i] *= rate;
	}
	
	pchassis->wheel_rpm[0]=wheel_rpm[0];
	pchassis->wheel_rpm[1]=wheel_rpm[1];
	pchassis->wheel_rpm[2]=wheel_rpm[2];
	pchassis->wheel_rpm[3]=wheel_rpm[3];	
}

float calculate_the_degree(void )
{
	float degree;
	float radio;
	radio=(pchassis->wheelbase + pchassis->wheeltrack) / 2.0f ;
	degree =(motor_info[0].all_angle-motor_info[1].all_angle-motor_info[2].all_angle+motor_info[3].all_angle)*360/(radio*8191);
	//����ƫת�ǲ�ת��Ϊ�Ƕ�
	while(degree>360) degree-=360;
	while(degree<-360) degree-=360;
	degree=degree*PI/180;//��Ϊ����
	return degree;
	
}











