#include "chassis_task.h"
#include "bsp_can.h"
#include "pid.h"
#include "bsp_dbus.h"
#include <stdio.h>
#include "math.h"

struct chassis schassis;
chassis_t pchassis = &schassis;
pid_struct_t chassis_motor_pid[4];
float vx, vy, wz;		//底盘速度
float location_vx,location_vy;//相对原坐标系的速度

void chassis_init(void)//初始化 速度 加速度 位置 四轮PID
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
		
	pchassis->wheel_perimeter=152*PI; //这就是轮子周长
	pchassis->wheeltrack=538;
	pchassis->wheelbase=509;
	
  pid_init(&chassis_motor_pid[0],25,0.05,0,2000,INIT_current);//INIT_current为最大输出电流
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
      if (rc_device_get_state(&rc, RC_S2_UP))				//若sw2为UP，底盘跟随云台，速度由遥控器ch1-2控制，角速度PID由follow_relative_angle决定
      {//相当于右摇杆的两个通道控制x,y
				//(1684-364)/2
        vx = (float)rc.ch2 / 660 *MAX_CHASSIS_VX_SPEED;   //ch2控制x方向速度
        vy = (float)rc.ch1 / 660 *MAX_CHASSIS_VX_SPEED;   //ch1控制y方向速度
        wz =-(float)rc.wheel/ 660 *MAX_CHASSIS_VW_SPEED;	//follow_relative_angle更新于gimbal_info_rcv				
      }//角速度应该是由云台的板子控制
      if (rc_device_get_state(&rc, RC_S2_MID))			//若S2在中间,则由键盘控制
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
			if(rc_device_get_state(&rc, RC_S2_DOWN))//S2为DOWN时，开启小陀螺模式，仍然接收遥控数据
			{
				location_vx = (float)rc.ch2 / 660 *MAX_CHASSIS_VX_SPEED;   //ch2控制x方向速度
        location_vy = (float)rc.ch1 / 660 *MAX_CHASSIS_VX_SPEED;   //ch1控制y方向速度
        wz =2000;	//暂定转速为2000 
				degree=calculate_the_degree();
				vx=location_vx*cos(degree)+location_vy*sin(degree);
				vy=-location_vx*sin(degree)+location_vy*cos(degree);
			}
	chassis_set_speed(vx,vy,wz);
	chassis_execute();
	osDelayUntil(&period, 2);		
	}
}

void chassis_set_speed(float vx_mmps,float vy_mmps,float wz_degps) //这就是把得到的xv,yv,zw传到底盘指针中
{
	pchassis->xv_mm_s=vx_mmps;
	pchassis->yv_mm_s=vy_mmps;
	pchassis->zw_deg_s=wz_degps; 	
}
void chassis_execute()//底盘控制
{
	mecanum_calculate();	//底盘速度分解到四个轮子上
		for (int i = 0; i < 4; i++)		//连续给四个麦克纳姆轮输出
	{	
		chassis_motor_pid[i].output=pid_calc(&chassis_motor_pid[i],pchassis->wheel_rpm[i],motor_info[i].rotor_speed);		
	}
	set_motor_voltage(M3508M2006_TXID,chassis_motor_pid[0].output,chassis_motor_pid[1].output,chassis_motor_pid[2].output,chassis_motor_pid[3].output);
  //通过PID转化为电压输出
}

void mecanum_calculate(void)//底盘速度分解到四个轮子上	
{
	static float rotate_ratio_fr;//前右轮转速 
	static float rotate_ratio_fl;//前左轮转速
	static float rotate_ratio_bl;//后左轮转速
	static float rotate_ratio_br;//后右轮转速
	static float wheel_rpm_ratio;

	rotate_ratio_fr = ((pchassis->wheelbase + pchassis->wheeltrack) / 2.0f - pchassis->rotate_x_offset + pchassis->rotate_y_offset) / 57.3f;
	rotate_ratio_fl = ((pchassis->wheelbase + pchassis->wheeltrack) / 2.0f - pchassis->rotate_x_offset - pchassis->rotate_y_offset) / 57.3f;
	rotate_ratio_bl = ((pchassis->wheelbase + pchassis->wheeltrack) / 2.0f + pchassis->rotate_x_offset - pchassis->rotate_y_offset) / 57.3f;
	rotate_ratio_br = ((pchassis->wheelbase + pchassis->wheeltrack) / 2.0f + pchassis->rotate_x_offset + pchassis->rotate_y_offset) / 57.3f;
  //前面两项应该是车的长和宽,后面两项应该是车的中心偏移,除以57.3就相当于把度变成弧度
	wheel_rpm_ratio = 60.0f / (pchassis->wheel_perimeter * MOTOR_DECELE_RATIO);//60/(轮子周长*减速比)
  //这个没错编码器转速是更快一点

	MEC_VAL_LIMIT(pchassis->xv_mm_s, -MAX_CHASSIS_VX_SPEED, MAX_CHASSIS_VX_SPEED); //mm/s //限制一下各个速度的范围
	MEC_VAL_LIMIT(pchassis->yv_mm_s, -MAX_CHASSIS_VY_SPEED, MAX_CHASSIS_VY_SPEED); //mm/s
	MEC_VAL_LIMIT(pchassis->zw_deg_s, -MAX_CHASSIS_VW_SPEED, MAX_CHASSIS_VW_SPEED); //deg/s

	float wheel_rpm[4];
	float max = 0;
  //mm/s,弧度/s化为r/min
	wheel_rpm[0] = (pchassis->xv_mm_s - pchassis->yv_mm_s + pchassis->zw_deg_s * rotate_ratio_fr) * wheel_rpm_ratio;//相当于角速度*6.63,最外面乘了个常数
	wheel_rpm[1] = (pchassis->xv_mm_s + pchassis->yv_mm_s + pchassis->zw_deg_s * rotate_ratio_fl) * wheel_rpm_ratio;
	wheel_rpm[2] = (-pchassis->xv_mm_s + pchassis->yv_mm_s + pchassis->zw_deg_s * rotate_ratio_bl) * wheel_rpm_ratio;
	wheel_rpm[3] = (-pchassis->xv_mm_s - pchassis->yv_mm_s + pchassis->zw_deg_s * rotate_ratio_br) * wheel_rpm_ratio;
  //将单位化为r/min
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
	//计算偏转角并转化为角度
	while(degree>360) degree-=360;
	while(degree<-360) degree-=360;
	degree=degree*PI/180;//化为弧度
	return degree;
	
}











