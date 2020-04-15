#ifndef GIMBAL_TASK_H
#define GIMBAL_TASK_H

#include "bsp_can.h"
#include "pid.h"

#define YAW_SPEED_PID 			0
#define YAW_POSITION_PID 		1
#define PITCH_SPEED_PID 		2
#define PITCH_POSITION_PID 	3


//�������ֵ����Լ���ֵ
#define HALF_ECD_RANGE  4096
#define ECD_RANGE       8191
//��̨У׼��ֵ��ʱ�򣬷���ԭʼ����ֵ���Լ���תʱ�䣬ͨ���������ж϶�ת
#define GIMBAL_CALI_MOTOR_SET   10
#define GIMBAL_CALI_STEP_TIME   2000
#define GIMBAL_CALI_GYRO_LIMIT  0.1f

#define GIMBAL_CALI_PITCH_MAX_STEP  1
#define GIMBAL_CALI_PITCH_MIN_STEP  2
#define GIMBAL_CALI_YAW_MAX_STEP    3
#define GIMBAL_CALI_YAW_MIN_STEP    4

#define GIMBAL_CALI_START_STEP  GIMBAL_CALI_PITCH_MAX_STEP
#define GIMBAL_CALI_END_STEP    5

		
typedef struct gimbal *gimbal_t;
struct gimbal_angle //��ǵ��ǽǶ�
{
	float yaw;
	float pitch;
};

struct gimbal_rate  //��ǵ�Ӧ�����ٶ�
{
	int16_t yaw_rate;
	int16_t pitch_rate;
};

struct gimbal_info
{
	uint8_t mode;
	struct gimbal_angle encoder_angle;
	struct gimbal_angle gyro_angle;
};

struct gimbal_init_t  //��̨yaw��pitch�����ƫ�ú������Сֵ
{
  float Yaw_offset;			
  float YawMin ;     
  float YawMax ;     
  float Pitch_offset;		
  float PitchMin	;	
  float PitchMax;	
  uint8_t step;		
};

struct gimbal
{
	uint8_t cali_compiete_flag;
	struct gimbal_init_t gimbal_init; 
	
	struct gimbal_angle encoder_target_angle;
	struct gimbal_angle gyro_target_angle;
	
	struct gimbal_rate encoder_target_rate;
	struct gimbal_rate gyro_target_rate;   

  struct gimbal_angle PC_target_angle;
	
	pid_struct_t gimbal_pid[4];
	
};

extern gimbal_t pgimbal;

void gimbal_init(gimbal_t pgimbal);
void gimbal_task(void const * argument);
void gimbal_control(gimbal_t pgimbal);
void gimbal_set_pitch_delta(float pitch);		//��pit��Ŀ��Ƕ�����pitchֵ
void gimbal_set_yaw_delta(float yaw);				//�ı���̨yaw��Ŀ��Ƕ�






#endif

