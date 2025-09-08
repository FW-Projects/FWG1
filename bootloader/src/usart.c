/**
  **************************************************************************
  * @file     usart.c
  * @brief    usart program
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */
#include "stdbool.h"
#include "usart.h"
#include "flash.h"
#include "tmr.h"
#include "wk_crc.h"
#include "iap.h"

/** @addtogroup UTILITIES_examples
  * @{
  */

/** @addtogroup USART_iap_bootloader
  * @{
  */
  void usart1_send_ok(void);
void usart1_send_error(void);
void copy_uint8_array(uint8_t* dest, const uint8_t* src, size_t start_offset, size_t length);
uint32_t convert_data(const uint8_t *input_array, uint32_t *output_array,
                      uint32_t start_index, uint32_t end_index);


bool iap_done = false;

uint8_t usart1_tx_buff[22];
uint32_t usart1_crc_buff[258];
uint8_t usart1_rx_buff[1036];
uint8_t pro_buff[1024];

uint32_t flash_add = 0x08004000;


usart_group_type usart_group_struct;

/**
  * @brief  init usart.
  * @param  none
  * @retval none
  */
void uart_init(uint32_t baudrate)
{
    gpio_init_type gpio_init_struct;
    /* enable the usart and it's io clock */
    crm_periph_clock_enable(CRM_USART1_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
    /* set default parameter */
    gpio_default_para_init(&gpio_init_struct);
    /* configure the usart1_tx  pa9 */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins = GPIO_PINS_9;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init(GPIOA, &gpio_init_struct);
    /* configure the usart1_rx  pa10 */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pins = GPIO_PINS_10;
    gpio_init_struct.gpio_pull = GPIO_PULL_UP;
    gpio_init(GPIOA, &gpio_init_struct);
    /*configure usart nvic interrupt */
    nvic_irq_enable(USART1_IRQn, 0, 0);
    /*configure usart param*/
    usart_init(USART1, baudrate, USART_DATA_8BITS, USART_STOP_1_BIT);
    usart_transmitter_enable(USART1, TRUE);
    usart_receiver_enable(USART1, TRUE);
    usart_interrupt_enable(USART1, USART_RDBF_INT, TRUE);
    usart_enable(USART1, TRUE);
}

/**
  * @brief  usart1 interrupt handler.
  * @param  none
  * @retval none
  */
void USART1_IRQHandler(void)
{
    static uint16_t reval = 0;
    static uint16_t count = 0;
    //static uint8_t number = 0;
    time_ira_cnt = 0;  /* clear upgrade time out flag */

    if (usart_interrupt_flag_get(USART1, USART_RDBF_FLAG) != RESET)
    {
        reval = usart_data_receive(USART1);
        usart1_rx_buff[count] = reval;
        count++;

        if (count == 1036)
        {
            count = 0;
#if 0
            /* check data */
            copy_uint8_array(pro_buff, usart1_rx_buff, 7, 1024);
            flash_1kb_write(flash_add, pro_buff);
            flash_add = flash_add + 0x400;
            //usart1_send_ok();
            number++;

            if (number > 7)
            {
                /* check app starting address whether 0x08xxxxxx */
                if (((*(uint32_t *)(0x08004000 + 4)) & 0xFF000000) == 0x08000000)
                {
                    flash_word_program(IAP_UPGRADE_FLAG_ADDR, IAP_UPGRADE_FLAG);
                    crm_reset();
                    /* jump and run in app */
                    app_load(0x08004000);
                }
            }

#endif

            /* check data */
            if (usart1_rx_buff[0] == 0xD1 && usart1_rx_buff[1035] == 0xF0)
            {
				convert_data(usart1_rx_buff,usart1_crc_buff,1,1030);
                uint32_t crc_value = crc_block_calculate(usart1_crc_buff, 258);
                crc_data_reset();

                if (usart1_rx_buff[1031] == ((crc_value >> 24) & 0xff)  &&
                        usart1_rx_buff[1032] == ((crc_value >> 16) & 0xff)  &&
                        usart1_rx_buff[1033] == ((crc_value >> 8)  & 0xff)   &&
                        usart1_rx_buff[1034] == (crc_value & 0xff))
                {
                    /* write data to app memory */
                    copy_uint8_array(pro_buff, usart1_rx_buff, 7, 1024);
                    flash_2kb_write(flash_add, pro_buff);
                    flash_add += 1024;
                    /* send ok */
                    usart1_send_ok();
                    /* the end */
                    if (usart1_rx_buff[2] == 0x04)
                    {
                        /* check app starting address whether 0x08xxxxxx */
                        if (((*(uint32_t *)(0x08004000 + 4)) & 0xFF000000) == 0x08000000)
                        {
							iap_done = true;
//							flash_word_program(IAP_UPGRADE_FLAG_ADDR, IAP_UPGRADE_FLAG);
//							/* disenable crc periph clock */
//                            crm_periph_clock_enable(CRM_CRC_PERIPH_CLOCK, FALSE);
//							crm_reset();
//                            /* jump and run in app */
//							
//                            app_load(0x08004000);
                        }
                    }
                }
                else
                {
                    /* send error */
                    usart1_send_error();
                }
            }
            else
            {
                /* send error */
                usart1_send_error();
            }
        }

        //    if(usart_group_struct.count > (USART_REC_LEN - 1))
        //    {
        //      usart_group_struct.count = 0;
        //      usart_group_struct.head = 0;
        //      usart_group_struct.tail = 0;
        //    }
        //    else
        //    {
        //      usart_group_struct.count++;
        //      usart_group_struct.buf[usart_group_struct.head++] = reval;
        //      if(usart_group_struct.head > (USART_REC_LEN - 1))
        //      {
        //        usart_group_struct.head = 0;
        //      }
        //    }
    }
}

/**
  * @}
  */
uint32_t convert_data(const uint8_t *input_array, uint32_t *output_array,
                      uint32_t start_index, uint32_t end_index)
{
    // 参数检查
    if (input_array == NULL || output_array == NULL)
    {
        return 0;
    }

    if (start_index > end_index)
    {
        return 0;
    }

    uint32_t byte_count = end_index - start_index + 1; // 要提取的字节数
    const uint8_t *ptr = &input_array[start_index]; // 指向第一个要提取的字节
    uint32_t i;
    uint32_t output_index = 0;

    // 处理完整的4字节块（大端模式）
    for (i = 0; i < byte_count / 4; i++)
    {
        output_array[output_index++] = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
        ptr += 4;
    }

    // 处理剩余字节（不足4字节时补零，大端模式）
    if (byte_count % 4 != 0)
    {
        uint32_t word = 0;
        uint8_t remaining = byte_count % 4;

        for (uint8_t j = 0; j < remaining; j++)
        {
            word |= ptr[j] << (24 - (j * 8)); // 第一个字节放在最高位
        }

        output_array[output_index++] = word;
    }

    return output_index; // 返回转换的32位数据数量
}

void copy_uint8_array(uint8_t* dest, const uint8_t* src, size_t start_offset, size_t length) {
    // 计算实际起始指针
    const uint8_t* src_start = src + start_offset;
    
    // 处理起始非对齐部分（按字节复制）
    while (((uintptr_t)src_start % sizeof(uint32_t)) != 0 && length > 0) {
        *dest++ = *src_start++;
        length--;
    }
    
    // 使用32位字复制提高效率
    if (length >= sizeof(uint32_t)) {
        size_t word_count = length / sizeof(uint32_t);
        uint32_t* dest32 = (uint32_t*)dest;
        const uint32_t* src32 = (const uint32_t*)src_start;
        
        // 字对齐复制
        for (size_t i = 0; i < word_count; i++) {
            *dest32++ = *src32++;
        }
        
        // 更新指针和剩余长度
        dest = (uint8_t*)dest32;
        src_start = (const uint8_t*)src32;
        length %= sizeof(uint32_t);
    }
    
    // 复制剩余字节
    for (size_t i = 0; i < length; i++) {
        *dest++ = *src_start++;
    }
}



void usart1_send_ok(void)
{
    uint32_t crc_value = 0;
    usart1_tx_buff[0] = 0xD1;
    usart1_tx_buff[1] = 0x02;
    usart1_tx_buff[2] = 0x02;
    usart1_tx_buff[3] = 0x00;
    usart1_tx_buff[4] = 0x00;
    usart1_tx_buff[5] = 0x00;
    usart1_tx_buff[6] = 0x0A;
    usart1_tx_buff[7] = 0x1D;
    usart1_tx_buff[8] = 0x00;
    usart1_tx_buff[9] = 0x00;
    usart1_tx_buff[10] = 0x00;
    usart1_tx_buff[11] = 0x00;
    usart1_tx_buff[12] = 0x00;
    usart1_tx_buff[13] = 0x00;
    usart1_tx_buff[14] = 0x00;
    usart1_tx_buff[15] = 0x00;
    usart1_tx_buff[16] = 0x00;
	
	convert_data(usart1_tx_buff,usart1_crc_buff,1,16);
	
	
//    usart1_crc_buff[0] = (((uint32_t)usart1_tx_buff [1] << 24) | ((uint32_t)usart1_tx_buff [2] << 16) |
//                          ((uint32_t)usart1_tx_buff [3] << 8)  | ((uint32_t)usart1_tx_buff [4]));
//    usart1_crc_buff[1] = (((uint32_t)usart1_tx_buff [5] << 24) | ((uint32_t)usart1_tx_buff [6] << 16) |
//                          ((uint32_t)usart1_tx_buff [7] << 8)  | ((uint32_t)usart1_tx_buff [8]));
//    usart1_crc_buff[2] = (((uint32_t)usart1_tx_buff [9] << 24) | ((uint32_t)usart1_tx_buff [10] << 16) |
//                          ((uint32_t)usart1_tx_buff [11] << 8)  | ((uint32_t)usart1_tx_buff [12]));
//    usart1_crc_buff[3] = (((uint32_t)usart1_tx_buff [13] << 24) | ((uint32_t)usart1_tx_buff [14] << 16) |
//                          ((uint32_t)usart1_tx_buff [15] << 8)  | ((uint32_t)usart1_tx_buff [16]));


    crc_value = crc_block_calculate(usart1_crc_buff, 4);
    crc_data_reset();
    usart1_tx_buff[17] = ((crc_value >> 24) & 0xff);
    usart1_tx_buff[18] = ((crc_value >> 16) & 0xff);
    usart1_tx_buff[19] = ((crc_value >> 8) & 0xff);
    usart1_tx_buff[20] = (crc_value & 0xff);
    usart1_tx_buff[21] = 0xF0;

    for (uint8_t i = 0; i < 22; i++)
    {
        usart_data_transmit(USART1, usart1_tx_buff[i ]);
        
        while (usart_flag_get(USART1, USART_TDC_FLAG) == RESET);
    }
}

void usart1_send_error(void)
{
    uint32_t crc_value = 0;
    usart1_tx_buff[0] = 0xD1;
    usart1_tx_buff[1] = 0x02;
    usart1_tx_buff[2] = 0x02;
    usart1_tx_buff[3] = 0x00;
    usart1_tx_buff[4] = 0x00;
    usart1_tx_buff[5] = 0x00;
    usart1_tx_buff[6] = 0x0A;
    usart1_tx_buff[7] = 0x18;
    usart1_tx_buff[8] = 0x00;
    usart1_tx_buff[9] = 0x00;
    usart1_tx_buff[10] = 0x00;
    usart1_tx_buff[11] = 0x00;
    usart1_tx_buff[12] = 0x00;
    usart1_tx_buff[13] = 0x00;
    usart1_tx_buff[14] = 0x00;
    usart1_tx_buff[15] = 0x00;
    usart1_tx_buff[16] = 0x00;
	convert_data(usart1_tx_buff,usart1_crc_buff,1,16);
//    usart1_crc_buff[0] = (((uint32_t)usart1_tx_buff [1] << 24) | ((uint32_t)usart1_tx_buff [2] << 16) |
//                          ((uint32_t)usart1_tx_buff [3] << 8)  | ((uint32_t)usart1_tx_buff [4]));
//    usart1_crc_buff[1] = (((uint32_t)usart1_tx_buff [5] << 24) | ((uint32_t)usart1_tx_buff [6] << 16) |
//                          ((uint32_t)usart1_tx_buff [7] << 8)  | ((uint32_t)usart1_tx_buff [8]));
//    usart1_crc_buff[2] = (((uint32_t)usart1_tx_buff [9] << 24) | ((uint32_t)usart1_tx_buff [10] << 16) |
//                          ((uint32_t)usart1_tx_buff [11] << 8)  | ((uint32_t)usart1_tx_buff [12]));
//    usart1_crc_buff[3] = (((uint32_t)usart1_tx_buff [13] << 24) | ((uint32_t)usart1_tx_buff [14] << 16) |
//                          ((uint32_t)usart1_tx_buff [15] << 8)  | ((uint32_t)usart1_tx_buff [16]));
    crc_value = crc_block_calculate(usart1_crc_buff, 4);
    crc_data_reset();
    usart1_tx_buff[17] = ((crc_value >> 24) & 0xff);
    usart1_tx_buff[18] = ((crc_value >> 16) & 0xff);
    usart1_tx_buff[19] = ((crc_value >> 8) & 0xff);
    usart1_tx_buff[20] = (crc_value & 0xff);
    usart1_tx_buff[21] = 0xF0;

    for (uint8_t i = 0; i < 22; i++)
    {
        usart_data_transmit(USART1, usart1_tx_buff[i ]);

        while (usart_flag_get(USART1, USART_TDC_FLAG) == RESET);
    }
}






/**
  * @}
  */
