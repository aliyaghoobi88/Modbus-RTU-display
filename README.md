# STM32 Modbus Display

## Short Description

This project implements a Modbus RTU slave for an STM32 microcontroller. It communicates with a Modbus master device over a UART interface and displays data on an SSD1306 OLED display. Key features include:

- **Modbus RTU Communication:** Initialization, enabling, and handling read/write operations on holding registers.
- **OLED Display Control:** Manages an SSD1306 OLED display, displaying text and graphics based on holding register values.
- **User Interface:** Provides a user interface with three keys (Up, Down, Enter) and a buzzer, controllable via holding registers.
- **EEPROM Storage:** Stores and retrieves configuration data (baud rate, Modbus ID) in EEPROM, modifiable through Modbus holding registers.
- **Holding Register Management:** Defines holding registers for storing display text, key states, buzzer control, and configuration settings.
![Untitled design](https://github.com/aliyaghoobi88/Modbus-RTU-display/assets/4157568/c5a98953-03ba-4128-b75e-823cef0f0ceb)

## Table of Contents

- [Config Timer](#config-timer)
- [Config LCD](#config-lcd)
- [Config EEPROM](#config-eeprom)
- [Key & Buzzer](#key--buzzer)
- [Display Initialization](#display-initialization)
- [Main Loop](#main-loop)
- [Modbus Register Table ](#Mbus-Table)
- [Python GUI](#guipython)
- [Roadmap](#roadmap)

## Config Timer

1. **Select the Timer:** Timer 3 (TIM3) is used for Modbus timing.
2. **Configure the Timer Prescaler:** Set to 71, resulting in a timer clock frequency of 1 MHz (assuming a system clock of 72 MHz).
3. **Set the Timer Period:** Set to 50, corresponding to an interrupt interval of 50 microseconds (50 μs).
4. **Set the Timer Counter Mode:** Set to `TIM_COUNTERMODE_UP`.
5. **Initialize the Timer:** Use `HAL_TIM_Base_Init` to initialize the timer.
6. **Pass the Timer Handle to Modbus:** Pass the configured timer handle (`htim3`) to `ModbusRtuInit`.

## Config LCD

1. **Configure I2C:** Set up I2C in STM32CubeMX for SSD1306 display communication.
2. **Select Font Size:** Choose a font size (e.g., `Font_11x18`) from the provided font files.
3. **Set Holding Register Addresses:** Set addresses and size based on font size and maximum characters.
4. **Update Holding Registers:** Update with the text to be displayed.
5. **Refresh Display:** Call `ssd1306_UpdateScreen` to refresh the OLED display.

Refer to [ssd1306-stm32HAL](https://github.com/4ilo/ssd1306-stm32HAL/tree/master) for more details.

## Config EEPROM

To store settings on the microcontroller, we use the EEPROM emulation library written by NimaLTD. The structure for storing information is defined in `rtudisplay.h`:

```c
typedef struct {
  USHORT Baudrate;
  USHORT ModbusID;
} Storage_t;
```
Refer to [EEPROM EMULATION Library for STM32](https://github.com/nimaltd/ee) for more details.

## Key & Buzzer

Provides a simple user interface with three keys (Up, Down, Enter) and a buzzer. Key states are stored in holding registers, and the buzzer is controlled via holding registers:

- **Key States:** Set corresponding holding register to 1 when a key is pressed.
- **Buzzer Control:** Write specific values to the holding register at `Buzzer_ADDR` to control the buzzer:
  - `1`: `buzzer_beep` (short beep)
  - `2`: `BUZZER_ON` (continuous on)
  - `3`: `BUZZER_OFF` (off)

## Display Initialization

The `Rtudisplay_Init` function initializes various components:

1. **Initialize Holding Registers:** Set `HRBuff` array to zeros.
2. **Initialize EEPROM:** Call `EE_Init` with `EE_Setting` structure.
3. **Read EEPROM Settings:** Call `EE_Read`.
4. **Set Default Values if Invalid:**
  - Baud rate default: 3 (9600)
  - Modbus ID default: 2
5. **Copy Settings to Holding Registers:**
  - `Baud_ADDR`
  - `MbID_ADDR`
6. **Set Baud Rate:** Based on `EE_Setting.Baudrate`.
7. **Initialize UART:** Call `HAL_UART_Init` with `huart2`.
8. **Initialize Modbus:** Call `ModbusRtuInit`.
9. **Initialize OLED Display:** Call `ssd1306_Init` with `hi2c1`.

## Main Loop

The main program loop (`while(1)`) performs the following:

1. **Handle Modbus Communication:** Call `eMBPoll()`.
2. **Update Display and Save Parameters:** If Write on Holding Register  function  received and `HR_Write_Flag` is  set.
3. **Update Buzzer State:** Based on holding register.
4. **Update Key States:** Reflect in holding registers.
This loop keeps the system responsive to Modbus, user input, and events requiring display or buzzer updates.

## Mbus-Table
 
| Address     	| Description   	|
|-------------	|:-----------------:|
| 40001-----40011 	| LCD Line 1        	|
| 40012-----40022 	| LCD Line 2        	|
| 40023-----40033 	| LCD Line 3        	|
| 40034-----40036 	| Reserved      	|
| 40037       	| Modbus ID     	|
| 40038       	| Baud Rate     	|
| 40039       	| Save Settings 	|
| 40040       	| Up Key        	|
| 40041       	| Down Key      	|
| 40042       	| Enter Key     	|
| 40043       	| Buzzer        	|

## guipython
This project offers a GUI for interacting with Modbus Dispaly RTU via Tkinter in Python, allowing users to configure communication, control a buzzer, and monitor keys and LEDs. Access the software [here](https://github.com/aliyaghoobi88/Modbus-Display-Config-Python).

![Gui](https://github.com/aliyaghoobi88/Modbus-RTU-display/assets/4157568/a23f8523-715b-4e05-b16a-05de5535a17b)

## roadmap

- [ ] The RS485 Directin Control pin declared at init function (Currently in Library\Modbus\rtu\mbrtu.c line 302,315  ) !!
- [ ] Support multi-size font for each line
- [ ] Add Changelog