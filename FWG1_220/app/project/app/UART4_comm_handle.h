#ifndef __UART4_COMM_HANDLE_H
#define __UART4_COMM_HANDLE_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "at32f415.h"
#include "at32_Usart.h"
#define UART4_HANDLE_TIME   2
#define UART4_USART 3
#define UART4_RX_BUFF_SIZE  256
#define UART4_TX_BUFF_SIZE  256

#define UART4_CRC_SIZE      4
#define UART4_MAX_SEND_SIZE 22
#define UART4_MAX_RECV_SIZE 22

#define UART4_CHECK_HEAD(A,B)         ((A==0xD1)&&(B==0xF0)?TRUE:FALSE)

#define UART4_HEAD1                0x00
#define UART4_CMD1                 0x01
#define UART4_CMD2                 0x02
#define UART4_ID_H                 0x03
#define UART4_ID_L                 0x04
#define UART4_DATA_LEN_H           0x05
#define UART4_DATA_LEN_L           0x06
#define UART4_DATA1_LEN_H          0x07
#define UART4_DATA1_LEN_L          0x08
#define UART4_DATA2_LEN_H          0x09
#define UART4_DATA2_LEN_L          0x0A
#define UART4_DATA3_LEN_H          0x0B
#define UART4_DATA3_LEN_L          0x0C
#define UART4_DATA4_LEN_H          0x0D
#define UART4_DATA4_LEN_L          0x0E
#define UART4_DATA5_LEN_H          0x0F
#define UART4_DATA5_LEN_L          0x10
#define UART4_CRC32_1              0x11
#define UART4_CRC32_2              0x12
#define UART4_CRC32_3              0x13
#define UART4_CRC32_4              0x14
#define UART4_HEAD2                0x15

typedef enum
{
    UART4_HEAD_1 = ((uint8_t)0xD1),
    UART4_HEAD_2 = ((uint8_t)0xF0),

} uart4_head_e;

typedef enum
{
	UART4_GENERAL = 0x01,
	UART4_FIRMWARE_UPDATE = 0x02,
    UART4_UI_UPDATE = 0x03,
}uart4_cmd1_e;

typedef enum
{
	UART4_GENERAL_ACK = 0x02,
	UART4_GENERAL_READ = 0x05,
	UART4_GENERAL_WRITE = 0x06,
	
	
}uart4_cmd2_e;

typedef enum
{
  UART4_SET_TEMP_ADD_EVENT,
  UART4_SET_TEMP_REDUCE_EVENT,
	
  UART4_SET_WIND_ADD_EVENT,
  UART4_SET_WIND_REDUCE_EVENT,
	
  UART4_GET_CHANNEL_1_EVENT,
  UART4_GET_CHANNEL_2_EVENT,
  UART4_GET_CHANNEL_3_EVENT,
  UART4_GET_CHANNEL_4_EVENT,

  	
  UART4_ENTER_NORMAL_MODE_EVENT,
  UART4_ENTER_COLD_MODE_EVENT,	
  
  UART4_SHOW_CURVE_PAGE_EVENT,
  UART4_SHOW_NORMAL_PAGE_EVENT,
	
  UART4_SET_TEMP_UNIT_C,
  UART4_SET_TEMP_UNIT_F,
	
  UART4_SELECT_HANDLE_EVENT,
	
  UART4_END_EVENT,
}uart4_event_e;





typedef struct UART4Object
{
    bool connect_flag;
    uint8_t rx_buff[UART4_RX_BUFF_SIZE];
    uint8_t read_size;
    uint8_t tx_buff[UART4_TX_BUFF_SIZE];
    uint32_t check_crc_buff[UART4_CRC_SIZE];

} UART4_DATA_t;
void uart4_comm_handle(void);

#endif


/*

G2风枪：
        帧头                 （1个）
产品信息：
        设备ID               （2个）
        软件版本号           （3个）
        设备序列号           （8个）
      
 

设备状态/参数：
				
        设备总运行时间       （2个）

        手柄1状态            （1个）
		手柄1工作时间        （2个）				     
		手柄1当前温度        （2个）
        手柄1当前风量        （2个）
        手柄1当前时间        （2个）
						     
        手柄1设置温度        （2个）
        手柄1设置风量        （2个）
        手柄1设置时间        （2个）
							 
        手柄1 通道1 设置温度 （2个）
        手柄1 通道1 设置风量 （2个）
        手柄1 通道1 设置时间 （2个）
							 
        手柄1 通道2 设置温度 （2个）
        手柄1 通道2 设置风量 （2个）
        手柄1 通道2 设置时间 （2个）
							 
        手柄1 通道3 设置温度 （2个）
        手柄1 通道3 设置风量 （2个）
        手柄1 通道3 设置时间 （2个）
							 
        手柄1 通道4 设置温度 （2个）
        手柄1 通道4 设置风量 （2个）
        手柄1 通道4 设置时间 （2个）
							 
        手柄2状态            （1个）
		手柄2工作时间        （2个）					 
		手柄2当前温度        （2个）
        手柄2当前风量        （2个）
        手柄2当前时间        （2个）
							 
        手柄2设置温度        （2个）
        手柄2设置风量        （2个）
        手柄2设置时间        （2个）
							 
        手柄2 通道1 设置温度 （2个）
        手柄2 通道1 设置风量 （2个）
        手柄2 通道1 设置时间 （2个）
							 
        手柄2 通道2 设置温度 （2个）
        手柄2 通道2 设置风量 （2个）
        手柄2 通道2 设置时间 （2个）
							 
        手柄2 通道3 设置温度 （2个）
        手柄2 通道3 设置风量 （2个）
        手柄2 通道3 设置时间 （2个）
							 
        手柄2 通道4 设置温度 （2个）
        手柄2 通道4 设置风量 （2个）
        手柄2 通道4 设置时间 （2个）

        心跳包时间戳         （2个）
        校验位1              （2个）
        校验位2              （2个）
        校验位3              （2个）
        校验位3              （2个）
        帧尾                 （1个）

*/
