/*
 * uart.h
 *
 *  Created on: Mar 11, 2024
 *      Author: kccistc
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "main.h"

typedef struct{
   uint8_t command;
   uint16_t data;
}protocol_t;

#define STX 0x02
#define ETX 0x03

void inituart(UART_HandleTypeDef *inHuart);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
int16_t getChar();
protocol_t receivepacket();
void transmitpacket(protocol_t data);
int _write(int file, char* p, int len);

#endif /* INC_UART_H_ */
