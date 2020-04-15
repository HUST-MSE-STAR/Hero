#ifndef __CHASSIS_TASK_H
#define __CHASSIS_TASK_H

#include "bsp_can.h"
#include "pid.h"
#include "cmsis_os.h"
#include "math.h"


/* the deceleration ratio of chassis motor */
#define MOTOR_DECELE_RATIO (1.0f / 19.0f) //这是减速比？
#define HIGH_RmeoteSpeed		660
#define MEC_VAL_LIMIT(val, min, max) \
  do                                 \
  {                                  \
    if ((val) <= (min))              \
    {                                \
      (val) = (min);                 \
    }                                \
    else if ((val) >= (max))         \
    {                                \
      (val) = (max);                 \
    }                                \
  } while (0);
  
#define MAX_CHASSIS_VX_SPEED 3000 //
#define MAX_CHASSIS_VY_SPEED 3000
#define MAX_CHASSIS_VW_SPEED 300 //5000rpm
#define INIT_current 3000 //	
#define MAX_WHEEL_RPM 9000 //9000rpm = 3732mm/s
#define PI 3.14159265354f
	

typedef struct chassis * chassis_t;
struct chassis
{
	float ax_mm_s2;//这应该是车子在x,y方向上的加速度,速度,位移
	float ay_mm_s2;
	float az_deg_s2;
	
	float xv_mm_s;
	float yv_mm_s;
	float zw_deg_s;
	
	float x_target;
	float y_target;
	float position_x_mm;
	float position_y_mm;
	float angle_deg;
	
	float wheel_rpm[4];//设定转速
	
	float wheel_perimeter; /* the perimeter(mm) of wheel */
	float wheeltrack;      /* wheel track distance(mm) */
	float wheelbase;       /* wheelbase distance(mm) */
	float rotate_x_offset; /* rotate offset(mm) relative to the x-axis of the chassis center */
	float rotate_y_offset; /* rotate offset(mm) relative to the y-axis of the chassis center */
	
};

extern pid_struct_t chassis_motor_pid[4];
extern chassis_t pchassis;


void chassis_init(void);    //初始化 速度 加速度 位置 四轮PID
void chassis_task(void const * argument);
void chassis_set_speed(float vx_mmps,float vy_mmps,float wz_degps);
void chassis_set_acc(float vx_mmps_acc,float vy_mmps_acc,float wz_degps_acc);
void chassis_set_offset(float offset_x, float offset_y);
void chassis_execute();//底盘控制
void mecanum_calculate(void);//底盘速度分解到四个轮子上
float calculate_the_degree(void );//计算坐标系偏转角



#endif

