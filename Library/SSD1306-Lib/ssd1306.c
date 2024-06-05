#include "ssd1306.h"
#include "math.h"
#include "stdlib.h"

// Screenbuffer
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

// Screen object
static SSD1306_t SSD1306;


//
//  Send a byte to the command register
//
static uint8_t ssd1306_WriteCommand(I2C_HandleTypeDef *hi2c, uint8_t command)
{
    return HAL_I2C_Mem_Write(hi2c, SSD1306_I2C_ADDR, 0x00, 1, &command, 1, 10);
}


//
//  Initialize the oled screen
//
uint8_t ssd1306_Init(I2C_HandleTypeDef *hi2c)
{
    // Wait for the screen to boot
    HAL_Delay(100);
    int status = 0;

    // Init LCD
    status += ssd1306_WriteCommand(hi2c, 0xAE);   // Display off
    status += ssd1306_WriteCommand(hi2c, 0x20);   // Set Memory Addressing Mode
    status += ssd1306_WriteCommand(hi2c, 0x10);   // 00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
    status += ssd1306_WriteCommand(hi2c, 0xB0);   // Set Page Start Address for Page Addressing Mode,0-7
    status += ssd1306_WriteCommand(hi2c, 0xC8);   // Set COM Output Scan Direction
    status += ssd1306_WriteCommand(hi2c, 0x00);   // Set low column address
    status += ssd1306_WriteCommand(hi2c, 0x10);   // Set high column address
    status += ssd1306_WriteCommand(hi2c, 0x40);   // Set start line address
    status += ssd1306_WriteCommand(hi2c, 0x81);   // set contrast control register
    status += ssd1306_WriteCommand(hi2c, 0xFF);
    status += ssd1306_WriteCommand(hi2c, 0xA1);   // Set segment re-map 0 to 127
    status += ssd1306_WriteCommand(hi2c, 0xA6);   // Set normal display

    status += ssd1306_WriteCommand(hi2c, 0xA8);   // Set multiplex ratio(1 to 64)
    status += ssd1306_WriteCommand(hi2c, SSD1306_HEIGHT - 1);

    status += ssd1306_WriteCommand(hi2c, 0xA4);   // 0xa4,Output follows RAM content;0xa5,Output ignores RAM content
    status += ssd1306_WriteCommand(hi2c, 0xD3);   // Set display offset
    status += ssd1306_WriteCommand(hi2c, 0x00);   // No offset
    status += ssd1306_WriteCommand(hi2c, 0xD5);   // Set display clock divide ratio/oscillator frequency
    status += ssd1306_WriteCommand(hi2c, 0xF0);   // Set divide ratio
    status += ssd1306_WriteCommand(hi2c, 0xD9);   // Set pre-charge period
    status += ssd1306_WriteCommand(hi2c, 0x22);

    status += ssd1306_WriteCommand(hi2c, 0xDA);   // Set com pins hardware configuration
    status += ssd1306_WriteCommand(hi2c, SSD1306_COM_LR_REMAP << 5 | SSD1306_COM_ALTERNATIVE_PIN_CONFIG << 4 | 0x02);   

    status += ssd1306_WriteCommand(hi2c, 0xDB);   // Set vcomh
    status += ssd1306_WriteCommand(hi2c, 0x20);   // 0x20,0.77xVcc
    status += ssd1306_WriteCommand(hi2c, 0x8D);   // Set DC-DC enable
    status += ssd1306_WriteCommand(hi2c, 0x14);   //
    status += ssd1306_WriteCommand(hi2c, 0xAF);   // Turn on SSD1306 panel

    if (status != 0) {
        return 1;
    }

    // Clear screen
    ssd1306_Fill(Black);

    // Flush buffer to screen
    ssd1306_UpdateScreen(hi2c);

    // Set default values for screen object
    SSD1306.CurrentX = 0;
    SSD1306.CurrentY = 0;

    SSD1306.Initialized = 1;

    return 0;
}

//
//  Fill the whole screen with the given color
//
void ssd1306_Fill(SSD1306_COLOR color)
{
    // Fill screenbuffer with a constant value (color)
    uint32_t i;

    for(i = 0; i < sizeof(SSD1306_Buffer); i++)
    {
        SSD1306_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
    }
}

//
//  Write the screenbuffer with changed to the screen
//
void ssd1306_UpdateScreen(I2C_HandleTypeDef *hi2c)
{
    uint8_t i;

    for (i = 0; i < 8; i++) {
        ssd1306_WriteCommand(hi2c, 0xB0 + i);
        ssd1306_WriteCommand(hi2c, 0x00);
        ssd1306_WriteCommand(hi2c, 0x10);

        HAL_I2C_Mem_Write(hi2c, SSD1306_I2C_ADDR, 0x40, 1, &SSD1306_Buffer[SSD1306_WIDTH * i], SSD1306_WIDTH, 100);
    }
}



void ssd1306_DrawRectangle(int x, int y, int width, int height, SSD1306_COLOR color) {
    // Draw the top and bottom lines
    for (int i = 0; i < width; i++) {
        ssd1306_DrawPixel(x + i, y, color);             // Top line
        ssd1306_DrawPixel(x + i, y + height - 1, color); // Bottom line
    }

    // Draw the left and right lines
    for (int i = 0; i < height; i++) {
        ssd1306_DrawPixel(x, y + i, color);             // Left line
        ssd1306_DrawPixel(x + width - 1, y + i, color); // Right line
    }
}


void ssd1306_DrawLine(int x0, int y0, int x1, int y1, SSD1306_COLOR color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        ssd1306_DrawPixel(x0, y0, color);  // Draw the current pixel

        if (x0 == x1 && y0 == y1) {
            break;  // Line is complete
        }

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void ssd1306_FillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, SSD1306_COLOR color) {
    int a, b, y, last;

    if (y0 > y1) { int tmp; tmp=y0; y0=y1; y1=tmp; tmp=x0; x0=x1; x1=tmp; }
    if (y1 > y2) { int tmp; tmp=y1; y1=y2; y2=tmp; tmp=x1; x1=x2; x2=tmp; }
    if (y0 > y1) { int tmp; tmp=y0; y0=y1; y1=tmp; tmp=x0; x0=x1; x1=tmp; }

    if (y0 == y2) {
        a = b = x0;
        if (x1 < a) a = x1;
        else if (x1 > b) b = x1;
        if (x2 < a) a = x2;
        else if (x2 > b) b = x2;
        ssd1306_DrawLine(a, y0, b, y0, color);
        return;
    }

    int dx01 = x1 - x0, dy01 = y1 - y0,
        dx02 = x2 - x0, dy02 = y2 - y0,
        dx12 = x2 - x1, dy12 = y2 - y1;
    int sa = 0, sb = 0;

    if (y1 == y2) last = y1;
    else last = y1 - 1;

    for (y = y0; y <= last; y++) {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        if (a > b) { int tmp=a; a=b; b=tmp; }
        ssd1306_DrawLine(a, y, b, y, color);
    }

    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for (; y <= y2; y++) {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        if (a > b) { int tmp=a; a=b; b=tmp; }
        ssd1306_DrawLine(a, y, b, y, color);
    }
}



void ssd1306_DrawTriangle(int x, int y, int base, int height, int direction, SSD1306_COLOR color, int fill) {
    int x1, y1, x2, y2, x3, y3;

    if (direction == 0) { // Upwards
        x1 = x;
        y1 = y;
        x2 = x + base;
        y2 = y;
        x3 = x + (base / 2);
        y3 = y - height;
    } else { // Downwards
        x1 = x;
        y1 = y;
        x2 = x + base;
        y2 = y;
        x3 = x + (base / 2);
        y3 = y + height;
    }

    if (fill) {
        ssd1306_FillTriangle(x1, y1, x2, y2, x3, y3, color);
    } else {
        ssd1306_DrawLine(x1, y1, x2, y2, color); // Draw base
        ssd1306_DrawLine(x1, y1, x3, y3, color); // Draw left side
        ssd1306_DrawLine(x2, y2, x3, y3, color); // Draw right side
    }
}


//
//  Draw one pixel in the screenbuffer
//  X => X Coordinate
//  Y => Y Coordinate
//  color => Pixel color
//
void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
    {
        // Don't write outside the buffer
        return;
    }

    // Check if pixel should be inverted
    if (SSD1306.Inverted)
    {
        color = (SSD1306_COLOR)!color;
    }

    // Draw in the correct color
    if (color == White)
    {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
    }
    else
    {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}


//
//  Draw 1 char to the screen buffer
//  ch      => Character to write
//  Font    => Font to use
//  color   => Black or White
//
char ssd1306_WriteChar(char ch, FontDef Font, SSD1306_COLOR color)
{
    uint32_t i, b, j;

    // Check remaining space on current line
    if (SSD1306_WIDTH <= (SSD1306.CurrentX + Font.FontWidth) ||
        SSD1306_HEIGHT <= (SSD1306.CurrentY + Font.FontHeight))
    {
        // Not enough space on current line
        return 0;
    }

    // Translate font to screenbuffer
    for (i = 0; i < Font.FontHeight; i++)
    {
        b = Font.data[(ch - 32) * Font.FontHeight + i];
        for (j = 0; j < Font.FontWidth; j++)
        {
            if ((b << j) & 0x8000)
            {
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR) color);
            }
            else
            {
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR)!color);
            }
        }
    }

    // The current space is now taken
    SSD1306.CurrentX += Font.FontWidth;

    // Return written char for validation
    return ch;
}

//
//  Write full string to screenbuffer
//
char ssd1306_WriteString(const char* str, FontDef Font, SSD1306_COLOR color)
{
    // Write until null-byte
    while (*str)
    {
        if (ssd1306_WriteChar(*str, Font, color) != *str)
        {
            // Char could not be written
            return *str;
        }

        // Next char
        str++;
    }

    // Everything ok
    return *str;
}

//
//  Invert background/foreground colors
//
void ssd1306_InvertColors(void)
{
    SSD1306.Inverted = !SSD1306.Inverted;
}

//
//  Set cursor position
//
void ssd1306_SetCursor(uint8_t x, uint8_t y)
{
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;
}
