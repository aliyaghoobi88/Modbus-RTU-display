/**
  ******************************************************************************
  * @file           : mt_port.h
  * @brief          : Additional porting data
  * @author         : MicroTechnics (microtechnics.ru)
  ******************************************************************************
  */

#ifndef MT_PORT_H
#define MT_PORT_H



/* Includes ------------------------------------------------------------------*/

#include "stm32F1xx_hal.h"



/* Declarations and definitions ----------------------------------------------*/




/* Functions -----------------------------------------------------------------*/

void MT_PORT_SetUartModule(UART_HandleTypeDef* uart);
void MT_PORT_SetTimerModule(TIM_HandleTypeDef* timer);
void ExitCriticalSection();
void EnterCriticalSection();



#endif // #ifndef MT_PORT_H
