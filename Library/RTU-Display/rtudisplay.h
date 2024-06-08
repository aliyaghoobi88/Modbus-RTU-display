#ifndef __RTUDISPLAY_H
#define __RTUDISPLAY_H


#include "stdint.h"
#include "stdbool.h"
#include "port.h"



#define DISPLAY_WIDTH   128
#define DISPLAY_HEIGHT  64

// Define font dimensions according to the selected font 
#define CHAR_WIDTH 11  
#define CHAR_HEIGHT 18 
#define CHARS_PER_LINE      (DISPLAY_WIDTH / CHAR_WIDTH)
#define TOTAL_LINES         (DISPLAY_HEIGHT / CHAR_HEIGHT)
#define LINE_BUFFER_SIZE    (CHARS_PER_LINE + 1) // +1 for null terminator



#define UP_KEY_STATE()          HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin)
#define Down_KEY_STATE()        HAL_GPIO_ReadPin(KEY4_GPIO_Port, KEY4_Pin)
#define Enter_KEY_STATE()       HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin)
#define BUZZER_ON()             HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET)
#define BUZZER_OFF()            HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET)

#define HR_REG_Size         52 /*for font 11*8  . Smaller font need More Holding register*/
#define HR_StartAdd         0


#define MbID_ADDR       (HR_StartAdd + 37)
#define Baud_ADDR       (HR_StartAdd + 38)
#define SaveCMD_ADDR    (HR_StartAdd + 39)
 


#define UP_KEY_ADDR     (HR_StartAdd + 40)
#define DOWN_KEY_ADDR   (HR_StartAdd + 41)
#define ENTER_KEY_ADDR  (HR_StartAdd + 42)
#define Buzzer_ADDR     (HR_StartAdd + 43)



typedef struct
{
 USHORT Baudrate;
 USHORT ModbusID;
}  Stotrage_t;


extern USHORT HRBuff[HR_REG_Size];

void Rtudisplay_UpdateLCD(void);
void Rtudisplay_UpdateKey(void);
void Rtudisplay_UpdateBuzzer(void);
void Rtudisplay_Init(void);
void Rtudisplay_SaveParam(void);

void buzzer_beep(void);

#endif  // _RTUDISPLAY_H


