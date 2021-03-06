/* 
 * bsp_dbus.c-dbus通信相关文件
 * NOTE: This file is based on HAL library of stm32 platform
 *       主要是debus通信协议的解析
 * Copyright (c) 2020-, FOSH Project
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:None
 */
#include "string.h"
#include "stdlib.h"
#include "bsp_dbus.h"
#include "usart.h"
#include "communication_task.h"
#include "cmsis_os.h"
#include "init.h"
#include "bsp_referee.h"

uint8_t   Referee_buf[REFEREE_BUFLEN];
uint8_t uartFlag;
uint8_t uartDir;
rc_info_t rc;
/**
  * @brief      enable global uart it and do not use DMA transfer done it
  * @param[in]  huart: uart IRQHandler id
  * @param[in]  pData: receive buff 
  * @param[in]  Size:  buff size
  * @retval     set success or fail
  */

static int uart_receive_dma_no_it(UART_HandleTypeDef* huart, uint8_t* pData, uint32_t Size)
{
  uint32_t tmp1 = 0;

  tmp1 = huart->RxState;
	
	if (tmp1 == HAL_UART_STATE_READY)
	{
		if ((pData == NULL) || (Size == 0))
		{
			return HAL_ERROR;
		}

//		huart->pRxBuffPtr = pData;
		huart->RxXferSize = Size;
		huart->ErrorCode  = HAL_UART_ERROR_NONE;

		/* Enable the DMA Stream */
	//要映射一个 DMA 通道以进行 USART 发送 ：   源地址 目标地址 要传输的数据数目            
		HAL_DMA_Start(huart->hdmarx, (uint32_t)&huart->Instance->RDR, (uint32_t)pData, Size);

		/*
		 * Enable the DMA transfer for the receiver request by setting the DMAR bit
		 * in the UART CR3 register 
		 */
		//激活该通道
		SET_BIT(huart->Instance->CR3, USART_CR3_DMAR);

		return HAL_OK;
	}
	else
	{
		return HAL_BUSY;
	}
}

/**
  * @brief      returns the number of remaining data units in the current DMAy Streamx transfer.
  * @param[in]  dma_stream: where y can be 1 or 2 to select the DMA and x can be 0
  *             to 7 to select the DMA Stream.
  * @retval     The number of remaining data units in the current DMAy Streamx transfer.
  */
//uint16_t dma_current_data_counter(DMA_Stream_TypeDef *dma_stream)
//{
//  /* Return the number of remaining data units for DMAy Streamx */
//  return ((uint16_t)(dma_stream->NDTR));
//}



/**
  * @brief       handle received rc data
  * @param[out]  rc:   structure to save handled rc data
  * @param[in]   buff: the buff which saved raw rc data
  * @retval 
  */
void rc_callback_handler(rc_info_t *rc, uint8_t *buff)
{
  rc->ch1 = (buff[0] | buff[1] << 8) & 0x07FF;//反正就是拼成16位,然后取11位
  rc->ch1 -= 1024;
  rc->ch2 = (buff[1] >> 3 | buff[2] << 5) & 0x07FF;
  rc->ch2 -= 1024;
  rc->ch3 = (buff[2] >> 6 | buff[3] << 2 | buff[4] << 10) & 0x07FF;
  rc->ch3 -= 1024;
  rc->ch4 = (buff[4] >> 1 | buff[5] << 7) & 0x07FF;
  rc->ch4 -= 1024;
	
	 /* prevent remote control zero deviation */
  if(rc->ch1 <= 5 && rc->ch1 >= -5)
    rc->ch1 = 0;
  if(rc->ch2 <= 5 && rc->ch2 >= -5)
    rc->ch2 = 0;
  if(rc->ch3 <= 5 && rc->ch3 >= -5)
    rc->ch3 = 0;
  if(rc->ch4 <= 5 && rc->ch4 >= -5)
    rc->ch4 = 0;
	
	rc->last_sw1=rc->sw1;
	rc->last_sw2=rc->sw2;
  rc->sw1 = ((buff[5] >> 4) & 0x000C) >> 2;
  rc->sw2 = (buff[5] >> 4) & 0x0003;
  
  if ((abs(rc->ch1) > 660) || \
      (abs(rc->ch2) > 660) || \
      (abs(rc->ch3) > 660) || \
      (abs(rc->ch4) > 660))
  {
    memset(rc, 0, sizeof(rc_info_t));
  }	
	rc->mouse.x = buff[6] | (buff[7] << 8); // x axis
  rc->mouse.y = buff[8] | (buff[9] << 8);
  rc->mouse.z = buff[10] | (buff[11] << 8);

  rc->mouse.l = buff[12];
  rc->mouse.r = buff[13];

  rc->key_code = buff[14] | buff[15] << 8; // key borad code
  rc->wheel = (buff[16] | buff[17] << 8) - 1024;
	
	get_dr16_state(rc);

	
}

static void get_dr16_state(rc_info_t *rc_dev)		
{
	
  rc_dev->state=0;
  if(rc_dev->sw1 == 3)
  {
    rc_dev->state |= RC_S1_MID;//打开位
    rc_dev->state &= ~RC_S1_UP;//关闭位
    rc_dev->state &= ~RC_S1_DOWN;
    if(rc_dev->last_sw1 == 1)
    {
      rc_dev->state |= RC_S1_UP2MID;
    }
    else if(rc_dev->last_sw1 == 2)
    {
      rc_dev->state |= RC_S1_DOWN2MID;
    }
  }
  else if(rc_dev->sw1 == 1)
  {
    rc_dev->state &= ~RC_S1_MID;
    rc_dev->state |= RC_S1_UP;
    rc_dev->state &= ~RC_S1_DOWN;
    if(rc_dev->last_sw1 == 3)
    {
      rc_dev->state |= RC_S1_MID2UP;
    }
  }
  else if(rc_dev->sw1 == 2)
  {
    rc_dev->state &= ~RC_S1_MID;
    rc_dev->state &= ~RC_S1_UP;
    rc_dev->state |= RC_S1_DOWN;
    if(rc_dev->last_sw1 == 3)
    {
      rc_dev->state |= RC_S1_MID2DOWN;
    }
  }
  
  if(rc_dev->sw2 == 3)
  {
    rc_dev->state |= RC_S2_MID;
    rc_dev->state &= ~RC_S2_UP;
    rc_dev->state &= ~RC_S2_DOWN;
    if(rc_dev->last_sw2 == 1)
    {
      rc_dev->state |= RC_S2_UP2MID;
    }
    else if(rc_dev->last_sw2 == 2)
    {
      rc_dev->state |= RC_S2_DOWN2MID;
    }
  }
  else if(rc_dev->sw2 == 1)
  {
    rc_dev->state &= ~RC_S2_MID;
    rc_dev->state |= RC_S2_UP;
    rc_dev->state &= ~RC_S2_DOWN;
    if(rc_dev->last_sw2 == 3)
    {
      rc_dev->state |= RC_S2_MID2UP;
    }
  }
  else if(rc_dev->sw2 == 2)
  {
    rc_dev->state &= ~RC_S2_MID;
    rc_dev->state &= ~RC_S2_UP;
    rc_dev->state |= RC_S2_DOWN;
    if(rc_dev->last_sw2 == 3)
    {
      rc_dev->state |= RC_S2_MID2DOWN;
    }
  }
}

int8_t rc_device_get_state(rc_info_t *rc_dev, uint16_t state)			//判断遥控器是否满足传入状态值
{
   if (rc_dev != NULL)
  {
    if((rc_dev->state & state) == state)		//若满足该传入状态值，则返回OK，并将该遥控器状态中的该状态位清零
    {return 1;}
    else																		//不满足则返回-RM_NOSTATE
    {return 0;}
  }
  return 0;
}


int8_t rc_device_get_key_press(rc_info_t *rc_dev, uint16_t key_press)			
{
   if (rc_dev != NULL)
  {
    if((rc_dev->key_code & key_press) == key_press)	
    {return 1;}
		else if((rc_dev->mouse.l&key_press)==key_press)
		{return 1;}
		else if((rc_dev->mouse.r&key_press)==key_press)
		{return 1;}		
    else																
    {return 0;}
  }
  return 0;
}

/**
  * @brief      clear idle it flag after uart receive a frame data
  * @param[in]  huart: uart IRQHandler id
  * @retval  
  */
static void uart_rx_idle_callback(UART_HandleTypeDef* huart)
{
	/* clear idle it flag avoid idle interrupt all the time */
	__HAL_UART_CLEAR_IDLEFLAG(huart);//清零IDLE标志，软件清零
	/* handle received data in idle interrupt */
	if (huart == &REFEREE_HUART)
	{
		/* clear DMA transfer complete flag */
		__HAL_DMA_DISABLE(huart->hdmarx);//禁止数据流

		/* handle dbus data dbus_buf from DMA */
		//if ((DBUS_MAX_LEN - dma_current_data_counter(huart->hdmarx->Instance)) == DBUS_BUFLEN)
		//{
			//referee_unpack_fifo_data();
		osSignalSet(CommunicationToReferee_taskhandle, RC_UPDATA);	
		//}
		
		/* restart dma transmission */
		//只有在禁止数据流时，才能向此NTDR执行写操作。 
    //使能数据流后，NTDR为只读，用于指示要传输的剩余数据项数。每次 DMA 传输后，此 
    //寄存器将递减。
		//__HAL_DMA_SET_COUNTER(huart->hdmarx, DBUS_MAX_LEN);//向此NTDR执行写操作
		__HAL_DMA_ENABLE(huart->hdmarx);
		//重新使能数据流
	}
	
}
uint8_t check_rev_buff(uint8_t *buff,const uint8_t len)
{
	return 1;
}


/**
  * @brief      callback this function when uart interrupt 
  * @param[in]  huart: uart IRQHandler id
  * @retval  
  */
void uart_receive_handler(UART_HandleTypeDef *huart)
{  
	if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) && 
			__HAL_UART_GET_IT_SOURCE(huart, UART_IT_IDLE))
	{	
		uart_rx_idle_callback(huart);
	}
}

/**
  * @brief   initialize dbus uart device 
  * @param   
  * @retval  
  */
void dbus_uart_init(void)
{
	/* open uart idle it */
	__HAL_UART_CLEAR_IDLEFLAG(&REFEREE_HUART);
	__HAL_UART_ENABLE_IT(&REFEREE_HUART, UART_IT_IDLE);//IDLE中断：当接收到1个字节，就会产生RXNE中断，当接收到一帧数据，就会产生IDLE中断。比如给单片机一次性发送了8个字节，就会产生8次RXNE中断，1次IDLE中断。）
	uart_receive_dma_no_it(&REFEREE_HUART, Referee_buf, REFEREE_MAX_LEN);
	
}










