#ifndef __RTUHANDLER_H
#define __RTUHANDLER_H

#include "stdint.h"
#include "usart.h"
#include "tim.h"
#include "port.h"
#include "mb.h"

#define REG_INPUT_START 5
#define REG_INPUT_NREGS 8


extern uint8_t HR_Write_Flag;
extern USHORT Holding_REG_Start;
extern USHORT HRBuff[];


extern void MT_PORT_SetUartModule(UART_HandleTypeDef *uart);
extern void MT_PORT_SetTimerModule(TIM_HandleTypeDef *timer);
uint8_t ModbusRtuInit(uint8_t uid,uint32_t Baudrate,TIM_HandleTypeDef* timer,UART_HandleTypeDef* uart);



#endif