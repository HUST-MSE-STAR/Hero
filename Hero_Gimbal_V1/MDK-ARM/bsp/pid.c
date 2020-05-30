/**
  * file			pid.c
  * version		V1.0.0
  * date			2016年11月11日17:21:36
  * brief   		对于PID， 反馈/测量习惯性叫get/measure/real/fdb,
						  期望输入一般叫set/target/ref*/
 #include "pid.h"
 
// pid_struct_t motor_pid[4];
/**
    * @brief  用于PID的初始化
    * @note   在PID初始化的时候调用
    * @author 周森V(1),钟午杰V(2)
    * @param  pid结构体,kp:比例项,ki:积分项,kd:微分项,i_max:积分最大值,out_max:输出最大值
    * @retval None
    */ 
void pid_init(pid_struct_t *pid,
              float kp,
              float ki,
              float kd,
              float i_max,
              float out_max)
{
  pid->kp      = kp;
  pid->ki      = ki;
  pid->kd      = kd;
  pid->i_max   = i_max;
  pid->out_max = out_max;
}

/**
    * @brief  用于PID的调用
    * @note   在位置环或速度环的时候调用
    * @author 周森V(1),钟午杰V(2)
	* @param  *pid:pid结构体,ref:期望值,fdb:反馈值
    * @retval PID的输出
    */
float pid_calc(pid_struct_t *pid, float ref, float fdb)
{
  pid->ref = ref;
  pid->fdb = fdb;
  pid->err[1] = pid->err[0];
  pid->err[0] = pid->ref - pid->fdb;
  
  pid->p_out  = pid->kp * pid->err[0];
  pid->i_out += pid->ki * pid->err[0];
  pid->d_out  = pid->kd * ( pid->err[0] - pid->err[1] );
  LIMIT_MIN_MAX(pid->i_out, -pid->i_max, pid->i_max);
  
  pid->output = pid->p_out + pid->i_out + pid->d_out;
  LIMIT_MIN_MAX(pid->output, -pid->out_max, pid->out_max);
  return pid->output;
}

void pid_tune(pid_struct_t *pid, float kp, float ki, float kd)
{
	pid->kp = kp;
	pid->ki = ki;
	pid->kd = kd;
}
  

