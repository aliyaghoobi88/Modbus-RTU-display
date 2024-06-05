#include "rtudisplay.h"
#include <stdio.h>
#include <string.h>
#include "ssd1306.h"
#include "fonts.h"
#include "i2c.h"
#include "gpio.h"
#include "main.h"


USHORT Holding_REG_Start = HR_StartAdd;
USHORT HRBuff[HR_REG_Size];
#define MAX_KEY_NAME_LENGTH 5 // Maximum length of key names
#define FONT_WIDTH 7
#define FONT_HEIGHT 10
#define BUZZER_DELAY_MS 100


void buzzer_beep()
{
      BUZZER_ON() ;
    HAL_Delay(70);
    BUZZER_OFF() ;
}




void update_display(void)
{
		OnlineValue_view((float)HRBuff[2]/1000,(float)HRBuff[3]/1000);
	
		if (Enter_KEY_STATE() == 0)
		{
			float value =MenuDensity_ctrl(HRBuff[4]/1000);
			HRBuff[4]=(USHORT)(value*1000);
		}



}


float MenuDensity_ctrl(float Value) {
    uint8_t exit_condition = 0;
    uint8_t current_part = 0; // 0 = thousandths, 1 = hundredths, 2 = tenths, 3 = integer
    uint32_t last_key_press = HAL_GetTick();
    uint32_t enter_key_press_time = 0;
    uint8_t enter_key_long_press_detected = 0;
    
    MenuDensity_view(Value, current_part);

    while (exit_condition == 0) {
        uint32_t current_time = HAL_GetTick();

        // Exit if no key pressed for more than 3 seconds
        if (current_time - last_key_press > 3000) {
            exit_condition = 1;
            break;
        }

        // Check for UP key press
        if (UP_KEY_STATE() == 0) {
            last_key_press = current_time;
            enter_key_long_press_detected = 0; // Reset long press flag for Enter key
            if (current_part == 0) {
                Value += 0.001;
                if (Value >= 10.0) Value -= 10.0;
            } else if (current_part == 1) {
                Value += 0.01;
                if (Value >= 10.0) Value -= 10.0;
            } else if (current_part == 2) {
                Value += 0.1;
                if (Value >= 10.0) Value -= 10.0;
            } else if (current_part == 3) {
                Value += 1.0;
                if (Value >= 10.0) Value -= 10.0;
            }
            MenuDensity_view(Value, current_part);
            buzzer_beep();
        }

        // Check for Down key press
        if (Down_KEY_STATE() == 0) {
            last_key_press = current_time;
            enter_key_long_press_detected = 0; // Reset long press flag for Enter key
            if (current_part == 0) {
                Value -= 0.001;
                if (Value < 0) Value += 10.0;
            } else if (current_part == 1) {
                Value -= 0.01;
                if (Value < 0) Value += 10.0;
            } else if (current_part == 2) {
                Value -= 0.1;
                if (Value < 0) Value += 10.0;
            } else if (current_part == 3) {
                Value -= 1.0;
                if (Value < 0) Value += 10.0;
            }
            MenuDensity_view(Value, current_part);
            buzzer_beep();
        }

        // Check for Enter key press
        if (Enter_KEY_STATE() == 0) {
            if (enter_key_press_time == 0) {
                enter_key_press_time = current_time; // Start timing the press
            } else if ((current_time - enter_key_press_time) > 1000) { // Check for long press (1 second)
                enter_key_long_press_detected = 1;
                exit_condition = 1;
                break;
            }
        } else {
            if (enter_key_press_time > 0 && !enter_key_long_press_detected) { // If key was released before long press was detected
                last_key_press = current_time;
                enter_key_press_time = 0; // Reset timing
                current_part = (current_part + 1) % 4;
                MenuDensity_view(Value, current_part);
                buzzer_beep();
            }
        }

        HAL_Delay(50); // Small delay to avoid bouncing
    }
		return Value;
}




void MenuDensity_view(float Value, uint8_t current_part) {
    // Clear the SSD1306 display
    ssd1306_Fill(Black);

    // Format the value to three decimal points and convert to string
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%.3f", Value);

    // Calculate the length of the text
    int textLength = strlen(buffer);

    // Get display dimensions
    int displayWidth = 128;
    int displayHeight = 64;

    // Font dimensions
    int charWidth = 16;  // Font_16x26 width per character
    int charHeight = 26; // Font_16x26 height

    // Unit font dimensions
    int unitCharWidth = 7;  // Font7x10 width per character
    int unitCharHeight = 10; // Font7x10 height

    // Unit string
    char unit[] = "kg/L";
    int unitLength = strlen(unit);
    int unitWidth = unitCharWidth * unitLength;

    // Calculate the position to center the text
    int textWidth = charWidth * textLength;
    int x = (displayWidth - textWidth) / 2;
    int y = (displayHeight - charHeight) / 2;

    // Write the value in the middle of the display
    ssd1306_SetCursor(x, y);
    ssd1306_WriteString(buffer, Font_16x26, White);

    // Calculate the position to display the unit
    int unitX = x + textWidth + 5; // 5 pixels space to the right of the text
    int unitY = y + (charHeight - unitCharHeight); // Align bottom of the unit with bottom of the text

    // Write the unit to the right bottom of the text
    ssd1306_SetCursor(unitX, unitY);
    ssd1306_WriteString(unit, Font_7x10, White);

  int arrowX;

    switch (current_part)
    {
    case 0 :
             arrowX = x + (4 * charWidth)+3; /*add +3 to center the arrow font width 16 arrow width 10 then (16*10)/2 */
        break;
    case 1 :
            arrowX = x + (3 * charWidth)+3;
    break;

        case 2 :
            arrowX = x + (2 * charWidth)+3;
    break;

        case 3 :
            arrowX = x + (0 * charWidth)+3;
    break;
  
    default:
        break;
    }



    int arrowYUp = y - 10; // Above the text
    int arrowYDown = y + charHeight + 5; // Below the text

    // Draw the up arrow
    draw_up_arrow(arrowX, arrowYUp);

    // Draw the down arrow
    draw_down_arrow(arrowX, arrowYDown);

    // Update the buffer to show on the LCD
    ssd1306_UpdateScreen(&hi2c1);
}



void Buzzer_On(void) {
    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET);
}

void Buzzer_Off(void) {
    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
}
 


void effect_rollup(void) {
    // Get display dimensions
    int displayWidth = 128;
    int displayHeight = 64;

    // Clear the display before starting the effect
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen(&hi2c1);

    // Draw lines from the bottom to the top
    for (int y = displayHeight; y >= 0; y -= 4) {
        for (int i = 0; i < 4; i++) {
            if (y - i >= 0) {
                ssd1306_DrawLine(0, y - i, displayWidth - 1, y - i, White);
            }
        }
        ssd1306_UpdateScreen(&hi2c1);
        HAL_Delay(2); // Adjust the delay to control the speed of the rolling effect
    }

    // Ensure the final display is completely filled
   // ssd1306_Fill(White);
    ssd1306_UpdateScreen(&hi2c1);
}


void underline_text(int x, int y, int width) {
    for (int i = 0; i < width; i++) {
        ssd1306_DrawPixel(x + i, y + 26, White); // Assuming Font_16x26
    }
}

void draw_up_arrow(int x, int y) {
    int arrowHeight = 5;
    int arrowWidth = 10;
    for (int i = 0; i < arrowHeight; i++) {
        for (int j = 0; j < arrowWidth - 2 * i; j++) {
            ssd1306_DrawPixel(x + i + j, y + i, White);
        }
    }
}

void draw_down_arrow(int x, int y) {
    int arrowHeight = 5;
    int arrowWidth = 10;
    for (int i = 0; i < arrowHeight; i++) {
        for (int j = 0; j < arrowWidth - 2 * i; j++) {
            ssd1306_DrawPixel(x + i + j, y + arrowHeight - i - 1, White);
        }
    }
}



void OnlineValue_view(float totalizer, float flow) {
    // Clear the SSD1306 display
    ssd1306_Fill(Black);

    // Get display dimensions
    int displayWidth = 128;
    int displayHeight = 64;

    // Font dimensions
    int charWidth16x26 = 16;  // Font_16x26 width per character
    int charHeight16x26 = 26; // Font_16x26 height

    int charWidth7x10 = 7;  // Font_7x10 width per character
    int charHeight7x10 = 10; // Font_7x10 height

    // Totalizer string formatting
    char totalizerBuffer[10];
    snprintf(totalizerBuffer, sizeof(totalizerBuffer), "%.1f", totalizer);
    int totalizerTextLength = strlen(totalizerBuffer);

    // Unit string for totalizer
    char totalizerUnit[] = "Ton";
    int totalizerUnitLength = strlen(totalizerUnit);
    int totalizerUnitWidth = charWidth7x10 * totalizerUnitLength;

    // Calculate the position to center the totalizer text in the upper 75% of the screen
    int totalizerTextWidth = charWidth16x26 * totalizerTextLength;
    int totalizerX = (displayWidth - (totalizerTextWidth + totalizerUnitWidth + 5)) / 2; // +5 for space between text and unit
    int totalizerY = (displayHeight * 3 / 4 - charHeight16x26) / 2;

    // Write the totalizer value
    ssd1306_SetCursor(totalizerX, totalizerY);
    ssd1306_WriteString(totalizerBuffer, Font_16x26, White);

    // Calculate the position for the unit
    int totalizerUnitX = totalizerX + totalizerTextWidth + 5;
    int totalizerUnitY = totalizerY + (charHeight16x26 - charHeight7x10); // Align bottom of the unit with bottom of the text

    // Write the totalizer unit
    ssd1306_SetCursor(totalizerUnitX, totalizerUnitY);
    ssd1306_WriteString(totalizerUnit, Font_7x10, White);

    // Draw a line separating the totalizer and flow sections
    int lineY = displayHeight * 3 / 4;
    ssd1306_DrawLine(0, lineY, displayWidth, lineY, White);

    // Flow string formatting
    char flowBuffer[10];
    snprintf(flowBuffer, sizeof(flowBuffer), "%.3f", flow);

    // Unit string for flow
    char flowUnit[] = "L/s";
    int flowUnitLength = strlen(flowUnit);
    int flowUnitWidth = charWidth7x10 * flowUnitLength;

    // Calculate the position to center the flow text in the lower 25% of the screen
    int flowTextLength = strlen(flowBuffer);
    int flowTextWidth = charWidth7x10 * flowTextLength;
    int flowX = (displayWidth - (flowTextWidth + flowUnitWidth + 5)) / 2;
    int flowY = lineY + (displayHeight / 4 - charHeight7x10) / 2;

    // Write the flow value
    ssd1306_SetCursor(flowX, flowY);
    ssd1306_WriteString(flowBuffer, Font_7x10, White);

    // Calculate the position for the unit
    int flowUnitX = flowX + flowTextWidth + 5;
    int flowUnitY = flowY;

    // Write the flow unit
    ssd1306_SetCursor(flowUnitX, flowUnitY);
    ssd1306_WriteString(flowUnit, Font_7x10, White);

    // Update the buffer to show on the LCD
    ssd1306_UpdateScreen(&hi2c1);
}

