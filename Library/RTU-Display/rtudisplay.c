#include "rtudisplay.h"
#include <stdio.h>
#include <string.h>
#include "ssd1306.h"
#include "fonts.h"
#include "i2c.h"
#include "gpio.h"
#include "main.h"
#include "ee.h"
#include "RtuHandler.h"


USHORT Holding_REG_Start = HR_StartAdd;
USHORT HRBuff[HR_REG_Size];
Stotrage_t EE_Setting;

extern UART_HandleTypeDef huart2;   /*Modbus Uart*/

extern uint16_t HRBuff[HR_REG_Size];
extern I2C_HandleTypeDef hi2c1;




void buzzer_beep(void)
{
    BUZZER_ON() ;
    HAL_Delay(70);
    BUZZER_OFF() ;
}


void Rtudisplay_SaveParam(void)
{
    if (HRBuff[SaveCMD_ADDR] == 1 ) {  

    EE_Setting.Baudrate=HRBuff[Baud_ADDR];
    EE_Setting.ModbusID =  HRBuff[MbID_ADDR];
    EE_Write();
    HRBuff[SaveCMD_ADDR] =0;
    buzzer_beep();
    }
}

void Rtudisplay_UpdateKey(void) {

    if (UP_KEY_STATE() == 0 && HRBuff[UP_KEY_ADDR] == 0 ) {  
        HRBuff[UP_KEY_ADDR] = 1;
    } 
    if (Down_KEY_STATE() == 0 && HRBuff[DOWN_KEY_ADDR] == 0) {  
        HRBuff[DOWN_KEY_ADDR] = 1;
    } 
    if (Enter_KEY_STATE() == 0 && HRBuff[ENTER_KEY_ADDR] == 0) {  
        HRBuff[ENTER_KEY_ADDR] = 1;
    }
}


void Rtudisplay_Init(void)
{
uint32_t baudRateValue;
memset(HRBuff, 0, sizeof(HRBuff));
EE_Init(&EE_Setting, sizeof(Stotrage_t));
EE_Read();

if (EE_Setting.Baudrate > 5)
{
    EE_Setting.Baudrate=3 ; /*Default Value */
        EE_Write();
}
if (EE_Setting.ModbusID > 255)
{
    EE_Setting.ModbusID=2 ; /*Default Value */
        EE_Write();
}


HRBuff[Baud_ADDR]=EE_Setting.Baudrate;
HRBuff[MbID_ADDR]=EE_Setting.ModbusID;


   switch(EE_Setting.Baudrate) {
        case 0:
            baudRateValue = 1200;
            break;
        case 1:
            baudRateValue = 2400;
            break;
        case 2:
            baudRateValue = 4800;
            break;
        case 3:
            baudRateValue = 9600;
            break;
        case 4:
            baudRateValue = 115200;
            break;
        default:
            baudRateValue = 9600; // Default to 9600 for variable baud rate
            break;
    }

  huart2.Instance = USART2;
  huart2.Init.BaudRate = baudRateValue;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }

ModbusRtuInit(EE_Setting.ModbusID,baudRateValue,&htim3,&huart2);

if (ssd1306_Init(&hi2c1) != 0)
{
    BUZZER_ON();  
}
else
{
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen(&hi2c1);
}


}

void Rtudisplay_UpdateBuzzer(void)
{
    if(HRBuff[Buzzer_ADDR] == 1)
    {
        buzzer_beep();
        HRBuff[Buzzer_ADDR]=0;
    }

    if(HRBuff[Buzzer_ADDR] == 2)
    {
      BUZZER_ON() ;
    }


    if(HRBuff[Buzzer_ADDR] == 3)
    {
      BUZZER_OFF() ;
    }
}






void Rtudisplay_UpdateLCD(void) {
    // Clear the display
    ssd1306_Fill(Black);

    // Initialize a buffer to hold the characters for each line
    char lineBuffer[LINE_BUFFER_SIZE]; // +1 for null terminator

    // Iterate over each line
    for (int line = 0; line < TOTAL_LINES; line++) {
        // Calculate the starting index in HRBuff for the current line
        int startIndex = (line * CHARS_PER_LINE) + 1; // Index starts from 1

        // Ensure we don't go out of bounds
        if (startIndex >= HR_REG_Size) break;

        // Copy characters from HRBuff to lineBuffer and format them using printf
        for (int i = 0; i < CHARS_PER_LINE; i++) {
            int currentIndex = startIndex + i;
            if (currentIndex < HR_REG_Size) {
                uint16_t value = HRBuff[currentIndex];
                lineBuffer[i] = (value == 0) ? ' ' : (char)value; // Replace 0 with space
            } else {
                lineBuffer[i] = ' '; // Fill with spaces if out of bounds
            }
        }

        // Null terminate the lineBuffer
        lineBuffer[CHARS_PER_LINE] = '\0';

        // Format the lineBuffer content
        char formattedBuffer[LINE_BUFFER_SIZE + 20]; // Extra space for formatting
        snprintf(formattedBuffer, sizeof(formattedBuffer), "%s", lineBuffer);

        // Set the cursor position for the current line
        int yPos = line * CHAR_HEIGHT;
        ssd1306_SetCursor(1, yPos);

        // Write the formatted line to the display
        ssd1306_WriteString(formattedBuffer, Font_11x18, White);
    }

    // Update the buffer to show on the LCD
    ssd1306_UpdateScreen(&hi2c1);
}

