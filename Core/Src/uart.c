/*
 * uart.c
 *
 *  Created on: Mar 11, 2024
 *      Author: kccistc
 */

#include "uart.h"
#include <stdio.h>

UART_HandleTypeDef *myHuart;

#define rxBufferMax 255

int rxBufferGp;      //get pointer (read)
int rxBufferPp;      //put pointer (write)
uint8_t rxBuffer[rxBufferMax];
uint8_t rxChar;


// init device

void inituart(UART_HandleTypeDef *inHuart)
{
   myHuart = inHuart;
   HAL_UART_Receive_IT(myHuart, &rxChar, 1);
}

//process received charactor

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
   rxBuffer[rxBufferPp++] = rxChar;
   rxBufferPp %= rxBufferMax;
   HAL_UART_Receive_IT(myHuart, &rxChar, 1);
}

//get charactor from buffer
int16_t getChar()
{
   int16_t result;
   if(rxBufferGp == rxBufferPp) return -1;// if buffer empty
   result = rxBuffer[rxBufferGp++];
   rxBufferGp %= rxBufferMax;
   return result;
}

int _write(int file, char* p, int len)
{
   HAL_UART_Transmit(myHuart, p, len, 10);
   return len;
}


//packet transmit
void transmitpacket(protocol_t data){
   //initial define
   uint8_t txBuffer[] = {STX,0,0,0,0,ETX};
   txBuffer[1] = data.command;
   txBuffer[2] = (data.data>>7) | 0x80;
   txBuffer[3] = (data.data & 0x7f)|0x80;
   //CRC
   txBuffer[4] = txBuffer[0]+txBuffer[1]+txBuffer[2]+txBuffer[3];
   // data trans
   HAL_UART_Transmit(myHuart, txBuffer, sizeof(txBuffer), 1);
   // data trans finish holding
   while(HAL_UART_GetState(myHuart) == HAL_UART_STATE_BUSY_TX
         ||HAL_UART_GetState(myHuart) == HAL_UART_STATE_BUSY_TX_RX);

}


//packet receive
protocol_t receivepacket(){
   protocol_t result;
   uint8_t buffer[6];
   uint8_t count = 0;
   uint32_t timeout;

   int16_t ch = getChar();
   memset(&result,0,sizeof(buffer));
   if(ch== STX){
      buffer[count++] = ch;
      timeout = HAL_GetTick(); //timeout startpoint
      while(ch != ETX){
         ch = getChar();
         if(ch!=-1){
            buffer[count++]= ch;
         }
         //timeout figure
         if(HAL_GetTick() - timeout >= 2) return result; // timeout = 2ms
      } // when timeout, buffer 0 return
      //CRC infect
      uint8_t crc=0;
      for(int i = 0 ; i<4; i++)
         crc+= buffer[i];
      if(crc != buffer[4]) return result;
      //parsing after receive end
      result.command = buffer[1];
      result.data = buffer[3]&0x7f;
      result.data |= (buffer[2]&0x7f)<<7;
   }
   return result;
}


