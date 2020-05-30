/* 
 * bullet_task.c-��������ļ�
 * NOTE: ���ڲ�������
 *
 * Copyright (c) 2020-, FOSH Project
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * C*/
#include "bullet_task.h"
#include "cmsis_os.h"
#include "bsp_dbus.h"
#include <stdlib.h>
#include "bsp_can.h"
struct shoot sshoot; //����shoot�ṹ��
shoot_t pshoot = &sshoot;
int32_t shoot_cnt = 0,first_angle=0;
pid_struct_t shoot_pid;//����λ�û�PID
int8_t shoot_begin=0,shoot_once=0,shoot_continue=0;
uint32_t shoot_mode_check=0;
/**
    * @brief  ���ڲ�������ĳ�ʼ��
    * @note   None
    * @author �����
    * @param  None
    * @retval None
    */
void shoot_init(shoot_t pshoot)//����������ٶȻ�
{

	pid_init(&shoot_pid, 	  10, 	0, 	   0, 	3000, 	10000); //�����ٶȻ�PID��ʼ��
	pshoot->speed_target=0;
	pshoot->angle_target=0;
	pshoot->turn_angle=turn_angle_delta;

}
/**
    * @brief  ���ڲ����Ŀ���
    * @note   ��S1������,ÿ2s����һ��
    * @author �����
    * @param  None
    * @retval None
    */
void Bullet_Task(void const * argument)//��������
{
	uint32_t period = osKernelSysTick();
		shoot_init(pshoot);
	while(1)
	{
		if(rc_device_get_state(&rc,RC_S2_UP))//ң�������ƻ������˶�
		{
			if(rc_device_get_state(&rc, RC_S1_MID2UP))//���S1���д���
			{	
				shoot_begin=1;        //��ʼ���
				
			}
			if(rc_device_get_state(&rc, RC_S1_MID2DOWN)&&(shoot_begin==1))//������俪ʼ���ҿ������д���
			{
				shoot_once=1; //���Ǳ���ſ�ʼ����һ��
        first_angle=motor_info[4].all_angle;		//��¼���̵ĳ�ʼ�Ƕ�	
			}
			if((shoot_once==1)&&(rc_device_get_state(&rc, RC_S1_DOWN)))//������������һ���ҿ��ش���
			{
				if(abs(motor_info[4].all_angle-first_angle)<5*turn_angle_delta)//����ת�ĽǶ�С��30Ȧ
				{
				  pshoot->speed_target=TurnSpeed;//
				  shoot_mode_check=get_time_ms();					
				}
				else  
				{
					pshoot->speed_target=0;//ת����30Ȧ��ͣ����
					if((get_time_ms()-shoot_mode_check)>2000)		//����2s��һ�ε�   
				     {
					     pshoot->speed_target=0;
							 shoot_continue=1;
							 shoot_once=0;
				     }	
			  }				    
			}
					
			if(shoot_continue)
			{//����������µ��о��÷���ͣ�������������
				if(rc_device_get_state(&rc, RC_S1_DOWN2MID))
				{
					shoot_continue=0;
					pshoot->speed_target=0;
				}
				else
				{
					pshoot->speed_target=TurnSpeed;

				}
			}
		}
//		printf("%d\r\n",motor_info[2].torque_current);
  bullet_block_check(pshoot);		//����Ƿ��ת
	bullet_control(pshoot);
	osDelayUntil(&period, 5);
}
	}
/**
    * @brief  ���ڲ����������
    * @note   PID�ٶȻ�
    * @author �����
    * @param  None
    * @retval None
    */
void bullet_control(shoot_t pshoot) 
{
	int id_range=0;//3508��2006�ĵ��
	motor_info[4].set_voltage=pid_calc(&shoot_pid,	pshoot->fric3,motor_info[4].rotor_speed);
  set_motor_voltage(id_range,motor_info[4].set_voltage,0,0,0);
		
}
/**
    * @brief  ���ڶ�ת���
    * @note   ����⵽�����������ֶ�ת��ͨ��������ת��������ת
    * @author �����
    * @param  None
    * @retval None
    */
void bullet_block_check(shoot_t pshoot)	
{
  static uint8_t first_block = 0;		//��ʼ��ת��ʶ
  static uint32_t check_time;					//��ʼ��תʱ��
  static uint8_t solve_block=0;  //��ʼ�����ת��ʶ
  static uint32_t solve_time;					//�����תʱ��
	
  if (abs(motor_info[4].torque_current) > BlockCurrent)	//�����������������ת����
  {
    if (first_block == 0)
    {
      first_block = 1;					//��ʼ��ת��ʶ��λ����ǿ�ʼ
      check_time = get_time_ms();	//��¼��ת���Կ�ʼʱ��
    }
    else if(get_time_ms() - check_time > 100)	//����ת��ʼ����ĳԤ����ʱʱ��
    {
      first_block = 0;					//��տ�ʼ��ת��ʶ
			solve_block=1;  //��ʼ�����ת��ʶ
      solve_time = get_time_ms();	//��¼ȷ����ת��ʼʱ��
			pshoot->speed_target=BlockSpeed;
    }
  }
  else
  {
    first_block = 0;		//һ�������������������ת���������������ʼ��ת��־λ
  }
	
	if(solve_block==1)
	{
	  if(get_time_ms() - solve_time > 300)
		{
			solve_block=0;		
			pshoot->speed_target=TurnSpeed; //������תʱת����
		}
		else 
			pshoot->speed_target=BlockSpeed; //��תʱ���ٷ�ת
		
	}
	
}
uint32_t get_time_ms(void)//���ص�ǰʱ��(ms)
{
         return HAL_GetTick();
}

