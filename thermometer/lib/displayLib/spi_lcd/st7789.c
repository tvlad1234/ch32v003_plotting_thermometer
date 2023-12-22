#include "spi_lcd.h"
#include "mipi_reg.h"

#define delay_ms(ms) (*(((lcd)->spidev)->delay_ms))(ms)
void lcd_select(spi_lcd *lcd);
void lcd_deselect(spi_lcd *lcd);

void lcd_reset(spi_lcd *lcd);
void lcd_send_command_list(spi_lcd *lcd, uint8_t *addr);
void lcd_send_command(spi_lcd *lcd, uint8_t cmd, uint8_t *data_ptr, uint16_t cnt);

const uint8_t generic_st7789[] = { // Init commands for 7789 screens
    9,                             //  9 commands in list:
    SWRESET, ST_CMD_DELAY,         //  1: Software reset, no args, w/delay
    150,                           //     ~150 ms delay
    SLPOUT, ST_CMD_DELAY,          //  2: Out of sleep mode, no args, w/delay
    10,                            //      10 ms delay
    COLMOD, 1 + ST_CMD_DELAY,      //  3: Set color mode, 1 arg + delay:
    0x55,                          //     16-bit color
    10,                            //     10 ms delay
    MADCTL, 1,                     //  4: Mem access ctrl (directions), 1 arg:
    0x08,                          //     Row/col addr, bottom-top refresh
    CASET, 4,                      //  5: Column addr set, 4 args, no delay:
    0x00,
    0, //     XSTART = 0
    0,
    240,      //     XEND = 240
    RASET, 4, //  6: Row addr set, 4 args, no delay:
    0x00,
    0, //     YSTART = 0
    320 >> 8,
    320 & 0xFF,          //     YEND = 320
    INVON, ST_CMD_DELAY, //  7: hack
    10,
    NORON, ST_CMD_DELAY,  //  8: Normal display on, no args, w/delay
    10,                   //     10 ms delay
    DISPON, ST_CMD_DELAY, //  9: Main screen turn on, no args, delay
    10};                  //    10 ms delay

void lcd_st7789_rotate(spi_lcd *lcd, uint8_t m)
{
    uint8_t madctl = 0;
    lcd->rotation = m & 3; // can't be higher than 3

    switch (lcd->rotation)
    {
    case 0:
        madctl = MADCTL_MX | MADCTL_MY | MADCTL_RGB;
        lcd->_xstart = lcd->_colstart;
        lcd->_ystart = lcd->_rowstart;
        lcd->width = lcd->window_width;
        lcd->height = lcd->window_height;
        break;
    case 1:
        madctl = MADCTL_MY | MADCTL_MV | MADCTL_RGB;
        lcd->_xstart = lcd->_rowstart;
        lcd->_ystart = lcd->_colstart2;
        lcd->height = lcd->window_width;
        lcd->width = lcd->window_height;
        break;
    case 2:
        madctl = MADCTL_RGB;
        lcd->_xstart = lcd->_colstart2;
        lcd->_ystart = lcd->_rowstart2;
        lcd->width = lcd->window_width;
        lcd->height = lcd->window_height;
        break;
    case 3:
        madctl = MADCTL_MX | MADCTL_MV | MADCTL_RGB;
        lcd->_xstart = lcd->_rowstart2;
        lcd->_ystart = lcd->_colstart;
        lcd->height = lcd->window_width;
        lcd->width = lcd->window_height;
        break;
    }

    lcd_send_command(lcd, MADCTL, &madctl, 1);
}

void lcd_st7789_init(spi_lcd *lcd, uint16_t width, uint16_t height)
{
    lcd->rotate_fun = lcd_st7789_rotate;

    if (width == 172 && height == 320)
    {
        // 1.47" display
        lcd->_rowstart = lcd->_rowstart2 = 0;
        lcd->_colstart = lcd->_colstart2 = 34;
    }
    else if (width == 240 && height == 280)
    {
        // 1.69" display
        lcd->_rowstart = 20;
        lcd->_rowstart2 = 0;
        lcd->_colstart = lcd->_colstart2 = 0;
    }
    else if (width == 135 && height == 240)
    {
        // 1.14" display
        lcd->_rowstart = lcd->_rowstart2 = (int)((320 - height) / 2);
        // This is the only device currently supported device that has different
        // values for lcd->_colstart & lcd->_colstart2. You must ensure that the extra
        // pixel lands in lcd->_colstart and not in lcd->_colstart2
        lcd->_colstart = (int)((240 - width + 1) / 2);
        lcd->_colstart2 = (int)((240 - width) / 2);
    }
    else
    {
        // 1.3", 1.54", and 2.0" displays
        lcd->_rowstart = (320 - height);
        lcd->_rowstart2 = 0;
        lcd->_colstart = lcd->_colstart2 = (240 - width);
    }

    lcd->window_width = width;
    lcd->window_height = height;
    
    lcd_select(lcd);
    lcd_reset(lcd);
    lcd_send_command_list(lcd, generic_st7789);
    lcd_rotate(lcd, 2);
}
