/* 
 * bullet_task.c-拨弹相关文件
 * NOTE: 用于拨弹控制
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
struct shoot sshoot; //创建shoot结构体
shoot_t pshoot = &sshoot;
int32_t shoot_cnt = 0,first_angle=0;
pid_struct_t shoot_pid;//拨盘位置环PID
int8_t shoot_begin=0,shoot_once=0,shoot_continue=0;
uint32_t shoot_mode_check=0;
/**
    * @brief  用于拨弹电机的初始化
    * @note   None
    * @author 钟午杰
    * @param  None
    * @retval None
    */
void shoot_init(shoot_t pshoot)//三个电机的速度环
{

	pid_init(&shoot_pid, 	  10, 	0, 	   0, 	3000, 	10000); //拨盘速度环PID初始化
	pshoot->speed_target=0;
	pshoot->angle_target=0;
	pshoot->turn_angle=turn_angle_delta;

}
/**
    * @brief  用于拨弹的控制
    * @note   由S1来控制,每2s发射一发
    * @author 钟午杰
    * @param  None
    * @retval None
    */
void Bullet_Task(void const * argument)//拨弹任务
{
	uint32_t period = osKernelSysTick();
		shoot_init(pshoot);
	while(1)
	{
		if(rc_device_get_state(&rc,RC_S2_UP))//遥控器控制机器人运动
		{
			if(rc_device_get_state(&rc, RC_S1_MID2UP))//如果S1由中打到上
			{	
				shoot_begin=1;        //开始射击
				
			}
			if(rc_device_get_state(&rc, RC_S1_MID2DOWN)&&(shoot_begin==1))//如果发射开始了且开关由中打下
			{
				shoot_once=1; //这是标记着开始发射一次
        first_angle=motor_info[4].all_angle;		//记录拨盘的初始角度	
			}
			if((shoot_once==1)&&(rc_device_get_state(&rc, RC_S1_DOWN)))//如果发射完成了一次且开关打下
			{
				if(abs(motor_info[4].all_angle-first_angle)<5*turn_angle_delta)//拨盘转的角度小于30圈
				{
				  pshoot->speed_target=TurnSpeed;//
				  shoot_mode_check=get_time_ms();					
				}
				else  
				{
					pshoot->speed_target=0;//转到了30圈就停下来
					if((get_time_ms()-shoot_mode_check)>2000)		//控制2s发一次弹   
				     {
					     pshoot->speed_target=0;
							 shoot_continue=1;
							 shoot_once=0;
				     }	
			  }				    
			}
					
			if(shoot_continue)
			{//如果开关由下到中就让发射停下来，否则继续
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
  bullet_block_check(pshoot);		//检测是否堵转
	bullet_control(pshoot);
	osDelayUntil(&period, 5);
}
	}
/**
    * @brief  用于拨弹电机控制
    * @note   PID速度环
    * @author 钟午杰
    * @param  None
    * @retval None
    */
void bullet_control(shoot_t pshoot) 
{
	int id_range=0;//3508或2006的电机
	motor_info[4].set_voltage=pid_calc(&shoot_pid,	pshoot->fric3,motor_info[4].rotor_speed);
  set_motor_voltage(id_range,motor_info[4].set_voltage,0,0,0);
		
}
/**
    * @brief  用于堵转检测
    * @note   若检测到电流过大，则发现堵转，通过反向旋转来消除堵转
    * @author 钟午杰
    * @param  None
    * @retval None
    */
void bullet_block_check(shoot_t pshoot)	
{
  static uint8_t first_block = 0;		//开始堵转标识
  static uint32_t check_time;					//开始堵转时间
  static uint8_t solve_block=0;  //开始处理堵转标识
  static uint32_t solve_time;					//处理堵转时间
	
  if (abs(motor_info[4].torque_current) > BlockCurrent)	//拨弹电机电流超过堵转电流
  {
    if (first_block == 0)
    {
      first_block = 1;					//开始堵转标识置位，标记开始
      check_time = get_time_ms();	//记录堵转测试开始时间
    }
    else if(get_time_ms() - check_time > 100)	//当堵转开始超过某预定超时时间
    {
      first_block = 0;					//清空开始堵转标识
			solve_block=1;  //开始处理堵转标识
      solve_time = get_time_ms();	//记录确定堵转开始时间
			pshoot->speed_target=BlockSpeed;
    }
  }
  else
  {
    first_block = 0;		//一旦拨弹电机电流降至堵转电流以下则清除开始堵转标志位
  }
	
	if(solve_block==1)
	{
	  if(get_time_ms() - solve_time > 300)
		{
			solve_block=0;		
			pshoot->speed_target=TurnSpeed; //结束堵转时转回来
		}
		else 
			pshoot->speed_target=BlockSpeed; //堵转时快速反转
		
	}
	
}
uint32_t get_time_ms(void)//返回当前时间(ms)
{
         return HAL_GetTick();
}

