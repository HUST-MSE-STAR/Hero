/* 
 * gimbal_task.c-云台的控制文件
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

struct gimbal sgimbal;//他的目的就是为了得到下面那个指针
gimbal_t pgimbal = &sgimbal;
float pit_delta, yaw_delta;		//两轴角度增量

/**
    * @brief  用于云台和PID的初始化
    * @note   None
    * @author 钟午杰
    * @param  云台结构体 pgimbal
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
	pid_init(&(pgimbal->gimbal_pid[PITCH_POSITION_PID]), 	1.5, 0, 	   0, 	3000, 	30000);   //PID跑位置环
	
	pgimbal->encoder_target_angle.pitch = 0; //初始位置
	pgimbal->encoder_target_angle.yaw = 0; 	
}

/**
    * @brief  用于云台的控制
    * @note   sw2为up,云台由遥控控制,sw2为down,云台由PC控制
    * @author 钟午杰
    * @param  None
    * @retval None
    */
void gimbal_task(void const * argument)  //云台任务
{
	uint32_t period = osKernelSysTick();
	gimbal_init(pgimbal);
	while(1)
	{
	if (rc_device_get_state(&rc, RC_S2_UP))				//若sw2为UP，底盘跟随云台，云台两轴角度由遥控器控制
   {
		 	pgimbal->gimbal_init.step=GIMBAL_CALI_START_STEP;
      pit_delta = -rc.ch4 ;		//遥控器通道4为pit轴增量（pit角速度）
      yaw_delta = -rc.ch3;		//遥控器通道3为yaw轴增量（yaw角速度)
      gimbal_set_pitch_delta(pit_delta);			//改变云台pit轴目标角度
      gimbal_set_yaw_delta(yaw_delta);				//改变云台yaw轴目标角度				
   }
   if(rc_device_get_state(&rc, RC_S2_DOWN))//s2为down,获取pc中传过来的角度
			{
       pgimbal->encoder_target_angle.pitch = pgimbal->PC_target_angle.pitch ;
       pgimbal->encoder_target_angle.yaw = pgimbal->PC_target_angle.yaw ;				
			}	 
	 		 
	 	gimbal_control(pgimbal);
		osDelayUntil(&period, 2);
	}			
}

/**
    * @brief  用于云台的PID控制
    * @note   Yaw和Pitch均是位置环
    * @author 钟午杰
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


void gimbal_set_pitch_delta(float pitch)		//将pit轴目标角度增加pitch值
{
  pgimbal->encoder_target_angle.pitch = pgimbal->encoder_target_angle.pitch+pitch;
}

void gimbal_set_yaw_delta(float yaw)				//改变云台yaw轴目标角度
{
	pgimbal->encoder_target_angle.yaw = pgimbal->encoder_target_angle.yaw+yaw;
}











