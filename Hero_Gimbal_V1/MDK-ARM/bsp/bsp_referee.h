#ifndef _H_BSP_REFEREE_H
#define _H_BSP_REFEREE_H
#include "usart.h"
#define REF_PROTOCOL_HEADER                 0xA5 //��ָ���ǵ�һ������ 
#define REF_PROTOCOL_HEADER_SIZE            sizeof(frame_header_t)
#define REF_PROTOCOL_CMD_SIZE               2
#define REF_PROTOCOL_CRC16_SIZE             2
#define REF_HEADER_CRC_LEN                  (REF_PROTOCOL_HEADER_SIZE + REF_PROTOCOL_CRC16_SIZE)
#define REF_HEADER_CRC_CMDID_LEN            (REF_PROTOCOL_HEADER_SIZE + REF_PROTOCOL_CRC16_SIZE + sizeof(uint16_t))//���uint16_tָ����cmdsize
#define REF_HEADER_CMDID_LEN                (REF_PROTOCOL_HEADER_SIZE + sizeof(uint16_t))

#define REF_PROTOCOL_FRAME_MAX_SIZE         128  //���崢�����Ϣ���128���ֽ�
#define REF_PROTOCOL_CMD_MAX_NUM            20

#define REFEREE_BUFLEN 128
extern uint8_t 	REFEREE_buf[REFEREE_BUFLEN];
typedef enum
{
  STEP_HEADER_SOF  = 0,
  STEP_LENGTH_LOW  = 1,
  STEP_LENGTH_HIGH = 2,
  STEP_FRAME_SEQ   = 3,
  STEP_HEADER_CRC8 = 4,
  STEP_DATA_CRC16  = 5,
} unpack_step_e;//��ѹ����

typedef struct
{
  uint8_t  sof;
  uint16_t data_length;
  uint8_t  seq;
  uint8_t  crc8;
} frame_header_t;
typedef struct
{
  uint16_t       data_len;
  uint8_t        protocol_packet[REF_PROTOCOL_FRAME_MAX_SIZE];
  unpack_step_e  unpack_step;
  uint16_t       index;
} unpack_data_t;

void referee_unpack_fifo_data(void);//����ϵͳͨ��Э���������
uint32_t ref_verify_crc8(uint8_t *p_msg, uint32_t len);
uint32_t ref_verify_crc16(uint8_t *p_msg, uint32_t len);
uint16_t ref_get_crc16(uint8_t *p_msg, uint16_t len, uint16_t crc16);
uint8_t ref_get_crc8(uint8_t *p_msg, uint32_t len, uint8_t crc8);
void referee_data_handler(uint8_t *p_frame);//����ʧ�ܴ�����

#endif
