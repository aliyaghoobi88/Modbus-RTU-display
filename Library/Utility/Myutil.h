#ifndef __MYUTIL_H
#define __MYUTIL_H

#include "usart.h"

#define BUFFER_SIZE     64 // Adjust the size according to your needs
#define LOG_UART_HANDLE huart1


typedef union {
    float Float;
    uint16_t Byte[2];
} FloatBytes;


extern  FloatBytes   F2Byte;
extern  char         buffer[BUFFER_SIZE];

void        LogPrint(const char *format, ...);
void        print_binary(uint32_t value, unsigned int bitLength);
void        Mb_SetFloat(uint8_t Base_address, float FValue, uint16_t *array);
float       Mb_GetFloat(uint8_t Base_address, uint16_t *array);
float       LinInterp(float raw_value, float slope, float intercept);
float       MapSignalFloat(float input, float inputMin, float inputMax, float outputMin, float outputMax);
uint16_t    DACmapSignal(float input, float inputMin, float inputMax, uint16_t outputMin, uint16_t outputMax);


#endif