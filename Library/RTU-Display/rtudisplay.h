#ifndef __RTUDISPLAY_H
#define __RTUDISPLAY_H


#include "stdint.h"
#include "stdbool.h"
#include "port.h"


#define UP_KEY_STATE()    HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin)
#define Down_KEY_STATE()    HAL_GPIO_ReadPin(KEY4_GPIO_Port, KEY4_Pin)
#define Enter_KEY_STATE()    HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin)
#define BUZZER_ON()    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET)
#define BUZZER_OFF()   HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET)


#define EE_Param_Size 70 /*it's equal to total size of HR_Config Address*/
#define EE_Save_Location 4


#define HR_StartAdd 0
#define HR_Config_Add 20
#define HR_REG_Size 90

#define EEPROM_START_ADDRESS  ((uint32_t)0x08080000) // Example address


#define TransducerUID  1150
#define Transducersw 20
#define Transducerhw 20





extern USHORT HRBuff[HR_REG_Size];
void MenuDensity_view(float Value, uint8_t current_part);
float MenuDensity_ctrl(float Value);
void effect_rollup(void) ;
void underline_text(int x, int y, int width);
void draw_up_arrow(int x, int y) ;
void draw_down_arrow(int x, int y) ;
void OnlineValue_view(float totalizer, float flow);
void update_display(void);
#endif  // _SSD1306_H


