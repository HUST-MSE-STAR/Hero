/* 
 * gimbal_task.c-��̨�Ŀ����ļ�
 * NOTE: This file is based on HAL library of stm32 platform
 *
 * Copyright (c) 2020-, FOSH Project
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:None
 */
#include "string.h"
#include "stdlib.h"
#include "gimbal_task.h"
#include "cmsis_os.h"
#include "bsp_dbus.h"
#include <stdio.h>
#include "main.h"
#include "init.h"

struct gimbal sgimbal;//����Ŀ�ľ���Ϊ�˵õ������Ǹ�ָ��
gimbal_t pgimbal = &sgimbal;
float pit_delta, yaw_delta;		//����Ƕ�����

/**
    * @brief  ������̨��PID�ĳ�ʼ��
    * @note   None
    * @author �����
    * @param  ��̨�ṹ�� pgimbal
    * @retval None
    */
void gimbal_init(gimbal_t pgimbal)
{
	pgimbal->gimbal_init.step=GIMBAL_CALI_START_STEP;
	pgimbal->cali_compiete_flag=0;
	pgimbal->gimbal_init.PitchMax=ECD_RANGE/4;
	pgimbal->gimbal_init.PitchMin=-ECD_RANGE/4;
	pgimbal->gimbal_init.YawMax=ECD_RANGE/4;
	pgimbal->gimbal_init.YawMin=-ECD_RANGE/4;
	pid_init(&(pgimbal->gimbal_pid[YAW_SPEED_PID]),    		80,  0.35, 0.01,	3000, 	30000);  //400 0.35 0 3000 30000 //170 0.45 0.25 
	pid_init(&(pgimbal->gimbal_pid[PITCH_SPEED_PID]),  		90 , 	0.6, 	 2.5, 	3000, 	30000);  // 300 0.6 2.5 //280
	pid_init(&(pgimbal->gimbal_pid[YAW_POSITION_PID]), 	  1.1, 	0, 	   0, 	3000, 	30000);     //0.5
	pid_init(&(pgimbal->gimbal_pid[PITCH_POSITION_PID]), 	1.5, 0, 	   0, 	3000, 	30000);   //PID��λ�û�
	
	pgimbal->encoder_target_angle.pitch = 0; //��ʼλ��
	pgimbal->encoder_target_angle.yaw = 0; 	
}

/**
    * @brief  ������̨�Ŀ���
    * @note   sw2Ϊup,��̨��ң�ؿ���,sw2Ϊdown,��̨��PC����
    * @author �����
    * @param  None
    * @retval None
    */
void gimbal_task(void const * argument)  //��̨����
{
	uint32_t period = osKernelSysTick();
	gimbal_init(pgimbal);
	while(1)
	{
	if (rc_device_get_state(&rc, RC_S2_UP))				//��sw2ΪUP�����̸�����̨����̨����Ƕ���ң��������
   {
		 	pgimbal->gimbal_init.step=GIMBAL_CALI_START_STEP;
      pit_delta = -rc.ch4 ;		//ң����ͨ��4Ϊpit��������pit���ٶȣ�
      yaw_delta = -rc.ch3;		//ң����ͨ��3Ϊyaw��������yaw���ٶ�)
      gimbal_set_pitch_delta(pit_delta);			//�ı���̨pit��Ŀ��Ƕ�
      gimbal_set_yaw_delta(yaw_delta);				//�ı���̨yaw��Ŀ��Ƕ�				
   }
   if(rc_device_get_state(&rc, RC_S2_DOWN))//s2Ϊdown,��ȡpc�д������ĽǶ�
			{
       pgimbal->encoder_target_angle.pitch = pgimbal->PC_target_angle.pitch ;
       pgimbal->encoder_target_angle.yaw = pgimbal->PC_target_angle.yaw ;				
			}	 
	 		 
	 	gimbal_control(pgimbal);
		osDelayUntil(&period, 2);
	}			
}

/**
    * @brief  ������̨��PID����
    * @note   Yaw��Pitch����λ�û�
    * @author �����
    * @param  None
    * @retval None
    */
void gimbal_control(gimbal_t pgimbal)
{
	   int id_range=0;
	  if ( pgimbal->encoder_target_angle.pitch > pgimbal->gimbal_init.PitchMax )
			pgimbal->encoder_target_angle.pitch = pgimbal->gimbal_init.PitchMax;
		if ( pgimbal->encoder_target_angle.pitch < pgimbal->gimbal_init.PitchMin )
			pgimbal->encoder_target_angle.pitch = pgimbal->gimbal_init.PitchMin;
		if ( pgimbal->encoder_target_angle.yaw > pgimbal->gimbal_init.YawMax )
			pgimbal->encoder_target_angle.yaw = pgimbal->gimbal_init.YawMax;
		if ( pgimbal->encoder_target_angle.yaw < pgimbal->gimbal_init.YawMin )
			pgimbal->encoder_target_angle.yaw = pgimbal->gimbal_init.YawMin;
		pgimbal->encoder_target_rate.yaw_rate 	= pid_calc(&(pgimbal->gimbal_pid[YAW_POSITION_PID]), 		pgimbal->encoder_target_angle.yaw, 				motor_info[4].all_angle );
		motor_info[4].set_voltage 							= pid_calc(&(pgimbal->gimbal_pid[YAW_SPEED_PID]), 			pgimbal->encoder_target_rate.yaw_rate, 		motor_info[4].rotor_speed);
		pgimbal->encoder_target_rate.pitch_rate = pid_calc(&(pgimbal->gimbal_pid[PITCH_POSITION_PID]), 	pgimbal->encoder_target_angle.pitch, 			motor_info[5].all_angle );
		motor_info[5].set_voltage 							= pid_calc(&(pgimbal->gimbal_pid[PITCH_SPEED_PID]), 		pgimbal->encoder_target_rate.pitch_rate, 	motor_info[5].rotor_speed);

    set_motor_voltage(id_range,motor_info[4].set_voltage,motor_info[5].set_voltage,0,0);
}


void gimbal_set_pitch_delta(float pitch)		//��pit��Ŀ��Ƕ�����pitchֵ
{
  pgimbal->encoder_target_angle.pitch = pgimbal->encoder_target_angle.pitch+pitch;
}

void gimbal_set_yaw_delta(float yaw)				//�ı���̨yaw��Ŀ��Ƕ�
{
	pgimbal->encoder_target_angle.yaw = pgimbal->encoder_target_angle.yaw+yaw;
}











