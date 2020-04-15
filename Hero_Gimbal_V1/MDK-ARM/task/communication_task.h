#ifndef __COMMUNICATION_H
#define __COMMUNICATION_H

#include "usart.h"
#include "bsp_dbus.h"

void calc_rev_buff(void);
void Communication_To_Pc_Task(void const * argument);
void Communication_To_Chasis_Task(void const * argument);
void Communication_To_Chasis(void const * argument);
#endif










