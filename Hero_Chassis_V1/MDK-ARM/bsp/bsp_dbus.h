#ifndef __BSP_DBUS_H__
#define __BSP_DBUS_H__

#include "usart.h"

#define UART_RX_DMA_SIZE (1024)
#define REFEREE_MAX_LEN     (128)
#define REFEREE_BUFLEN      (128)
#define REFEREE_HUART       huart1 /* for dji remote controler reciever */


/** 
  * @brief  remote control information
  */
#define RC_S1_UP2MID   (1 << 0u)
#define RC_S1_MID2UP   (1 << 1u)
#define RC_S1_DOWN2MID (1 << 2u)
#define RC_S1_MID2DOWN (1 << 3u)

#define RC_S2_UP2MID   (1 << 4u)
#define RC_S2_MID2UP   (1 << 5u)
#define RC_S2_DOWN2MID (1 << 6u)
#define RC_S2_MID2DOWN (1 << 7u)

#define RC_S1_UP       (1 << 8u)
#define RC_S1_MID      (1 << 9u)
#define RC_S1_DOWN     (1 << 10u)
#define RC_S2_UP       (1 << 11u)
#define RC_S2_MID      (1 << 12u)
#define RC_S2_DOWN     (1 << 13u)

#define RC_S2_S1_DOWN  (1<<14u)

#define RC_UPDATA 0x01U
#define UART_HEAD 						0xFFU
#define UART_INFANTRY 				0x10U
#define UART_CHASSIS2GIMBAL17 0x07U

#define UART_NEED_SOLVE (1)
#define UART_GIMBAL172CHASSIS 0x04U

/* ----------------------- PC Key Definition-------------------------------- */
#define KEY_PRESSED_W            ((uint16_t)1 << 0)
#define KEY_PRESSED_S            ((uint16_t)1 << 1)
#define KEY_PRESSED_A            ((uint16_t)1 << 2)
#define KEY_PRESSED_D            ((uint16_t)1 << 3)
#define KEY_PRESSED_SHIFT        ((uint16_t)1 << 4)
#define KEY_PRESSED_CTRL         ((uint16_t)1 << 5)
#define KEY_PRESSED_Q            ((uint16_t)1 << 6)
#define KEY_PRESSED_E            ((uint16_t)1 << 7)
#define KEY_PRESSED_R            ((uint16_t)1 << 8)
#define KEY_PRESSED_F            ((uint16_t)1 << 9)
#define KEY_PRESSED_G            ((uint16_t)1 << 10)
#define KEY_PRESSED_Z            ((uint16_t)1 << 11)
#define KEY_PRESSED_X            ((uint16_t)1 << 12)
#define KEY_PRESSED_C            ((uint16_t)1 << 13)
#define KEY_PRESSED_V            ((uint16_t)1 << 14)
#define KEY_PRESSED_B            ((uint16_t)1 << 15)
/* ----------------------- PC mouse Definition-------------------------------- */
#define MOUSE_LEFT_PRESSED       (uint8_t)1
#define MOUSE_RIGHT_PRESSED       (uint8_t)1

/* ----------------------- Data Struct ------------------------------------- */

typedef __packed struct
{
  /* rocker channel information */
  int16_t ch1;
  int16_t ch2;
  int16_t ch3;
  int16_t ch4;
	
  /* left and right lever information */
  uint8_t sw1;
  uint8_t sw2;
	
	uint8_t last_sw1;
  uint8_t last_sw2;
	
	uint16_t state;
	
	__packed struct 
  {
    int16_t x;
    int16_t y;
    int16_t z;

    uint8_t l;
    uint8_t r;
  } mouse;
  /* keyboard key information */
    uint16_t key_code;

  int16_t wheel;
} rc_info_t;

extern rc_info_t rc;


void uart_receive_handler(UART_HandleTypeDef *huart);
void dbus_uart_init(void);
static void get_dr16_state(rc_info_t *rc_dev);
int8_t rc_device_get_state(rc_info_t *rc_dev, uint16_t state);
void rc_callback_handler(rc_info_t *rc, uint8_t *buff);//定义遥控回调函数
int8_t rc_device_get_key_press(rc_info_t *rc_dev, uint16_t key_press);
#endif













