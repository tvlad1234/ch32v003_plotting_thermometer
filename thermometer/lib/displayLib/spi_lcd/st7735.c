#include "spi_lcd.h"
#include "mipi_reg.h"

#include "st7735.h"

#define delay_ms(ms) (*(((lcd)->spidev)->delay_ms))(ms)
void lcd_select(spi_lcd *lcd);
void lcd_deselect(spi_lcd *lcd);

const uint8_t Bcmd[] = {       // Init commands for 7735B screens
    18,                        // 18 commands in list:
    SWRESET, ST_CMD_DELAY,     //  1: Software reset, no args, w/delay
    50,                        //     50 ms delay
    SLPOUT, ST_CMD_DELAY,      //  2: Out of sleep mode, no args, w/delay
    255,                       //     255 = max (500 ms) delay
    COLMOD, 1 + ST_CMD_DELAY,  //  3: Set color mode, 1 arg + delay:
    0x05,                      //     16-bit color
    10,                        //     10 ms delay
    FRMCTR1, 3 + ST_CMD_DELAY, //  4: Frame rate control, 3 args + delay:
    0x00,                      //     fastest refresh
    0x06,                      //     6 lines front porch
    0x03,                      //     3 lines back porch
    10,                        //     10 ms delay
    MADCTL, 1,                 //  5: Mem access ctl (directions), 1 arg:
    0x08,                      //     Row/col addr, bottom-top refresh
    DISSET5, 2,                //  6: Display settings #5, 2 args:
    0x15,                      //     1 clk cycle nonoverlap, 2 cycle gate
                               //     rise, 3 cycle osc equalize
    0x02,                      //     Fix on VTL
    INVCTR, 1,                 //  7: Display inversion control, 1 arg:
    0x0,                       //     Line inversion
    PWCTR1, 2 + ST_CMD_DELAY,  //  8: Power control, 2 args + delay:
    0x02,                      //     GVDD = 4.7V
    0x70,                      //     1.0uA
    10,                        //     10 ms delay
    PWCTR2, 1,                 //  9: Power control, 1 arg, no delay:
    0x05,                      //     VGH = 14.7V, VGL = -7.35V
    PWCTR3, 2,                 // 10: Power control, 2 args, no delay:
    0x01,                      //     Opamp current small
    0x02,                      //     Boost frequency
    VMCTR1, 2 + ST_CMD_DELAY,  // 11: Power control, 2 args + delay:
    0x3C,                      //     VCOMH = 4V
    0x38,                      //     VCOML = -1.1V
    10,                        //     10 ms delay
    PWCTR6, 2,                 // 12: Power control, 2 args, no delay:
    0x11, 0x15,
    GMCTRP1, 16,            // 13: Gamma Adjustments (pos. polarity), 16 args + delay:
    0x09, 0x16, 0x09, 0x20, //     (Not entirely necessary, but provides
    0x21, 0x1B, 0x13, 0x19, //      accurate colors)
    0x17, 0x15, 0x1E, 0x2B, 0x04, 0x05, 0x02, 0x0E,
    GMCTRN1, 16 + ST_CMD_DELAY,                         // 14: Gamma Adjustments (neg. polarity), 16 args + delay:
    0x0B, 0x14, 0x08, 0x1E,                             //     (Not entirely necessary, but provides
    0x22, 0x1D, 0x18, 0x1E,                             //      accurate colors)
    0x1B, 0x1A, 0x24, 0x2B, 0x06, 0x06, 0x02, 0x0F, 10, //     10 ms delay
    CASET, 4,                                           // 15: Column addr set, 4 args, no delay:
    0x00, 0x02,                                         //     XSTART = 2
    0x00, 0x81,                                         //     XEND = 129
    RASET, 4,                                           // 16: Row addr set, 4 args, no delay:
    0x00, 0x02,                                         //     XSTART = 1
    0x00, 0x81,                                         //     XEND = 160
    NORON, ST_CMD_DELAY,                                // 17: Normal display on, no args, w/delay
    10,                                                 //     10 ms delay
    DISPON, ST_CMD_DELAY,                               // 18: Main screen turn on, no args, delay
    255},                                               //     255 = max (500 ms) delay

    Rcmd1[] = {                // 7735R init, part 1 (red or green tab)
        15,                    // 15 commands in list:
        SWRESET, ST_CMD_DELAY, //  1: Software reset, 0 args, w/delay
        150,                   //     150 ms delay
        SLPOUT, ST_CMD_DELAY,  //  2: Out of sleep mode, 0 args, w/delay
        255,                   //     500 ms delay
        FRMCTR1, 3,            //  3: Framerate ctrl - normal mode, 3 arg:
        0x01, 0x2C, 0x2D,      //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
        FRMCTR2, 3,            //  4: Framerate ctrl - idle mode, 3 args:
        0x01, 0x2C, 0x2D,      //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
        FRMCTR3, 6,            //  5: Framerate - partial mode, 6 args:
        0x01, 0x2C, 0x2D,      //     Dot inversion mode
        0x01, 0x2C, 0x2D,      //     Line inversion mode
        INVCTR, 1,             //  6: Display inversion ctrl, 1 arg:
        0x07,                  //     No inversion
        PWCTR1, 3,             //  7: Power control, 3 args, no delay:
        0xA2, 0x02,            //     -4.6V
        0x84,                  //     AUTO mode
        PWCTR2, 1,             //  8: Power control, 1 arg, no delay:
        0xC5,                  //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
        PWCTR3, 2,             //  9: Power control, 2 args, no delay:
        0x0A,                  //     Opamp current small
        0x00,                  //     Boost frequency
        PWCTR4, 2,             // 10: Power control, 2 args, no delay:
        0x8A,                  //     BCLK/2,
        0x2A,                  //     opamp current small & medium low
        PWCTR5, 2,             // 11: Power control, 2 args, no delay:
        0x8A, 0xEE, VMCTR1, 1, // 12: Power control, 1 arg, no delay:
        0x0E, INVOFF, 0,       // 13: Don't invert display, no args
        MADCTL, 1,             // 14: Mem access ctl (directions), 1 arg:
        0xC8,                  //     row/col addr, bottom-top refresh
        COLMOD, 1,             // 15: set color mode, 1 arg, no delay:
        0x05},                 //     16-bit color

    Rcmd2green[] = {        // 7735R init, part 2 (green tab only)
        2,                  //  2 commands in list:
        CASET, 4,           //  1: Column addr set, 4 args, no delay:
        0x00, 0x02,         //     XSTART = 0
        0x00, 0x7F + 0x02,  //     XEND = 127
        RASET, 4,           //  2: Row addr set, 4 args, no delay:
        0x00, 0x01,         //     XSTART = 0
        0x00, 0x9F + 0x01}, //     XEND = 159

    Rcmd2red[] = {   // 7735R init, part 2 (red tab only)
        2,           //  2 commands in list:
        CASET, 4,    //  1: Column addr set, 4 args, no delay:
        0x00, 0x00,  //     XSTART = 0
        0x00, 0x7F,  //     XEND = 127
        RASET, 4,    //  2: Row addr set, 4 args, no delay:
        0x00, 0x00,  //     XSTART = 0
        0x00, 0x9F}, //     XEND = 159

    Rcmd2green144[] = { // 7735R init, part 2 (green 1.44 tab)
        2,              //  2 commands in list:
        CASET, 4,       //  1: Column addr set, 4 args, no delay:
        0x00, 0x00,     //     XSTART = 0
        0x00, 0x7F,     //     XEND = 127
        RASET, 4,       //  2: Row addr set, 4 args, no delay:
        0x00, 0x00,     //     XSTART = 0
        0x00, 0x7F},    //     XEND = 127

    Rcmd2green160x80[] = { // 7735R init, part 2 (mini 160x80)
        3,                 //  3 commands in list:
        CASET, 4,          //  1: Column addr set, 4 args, no delay:
        0x00, 0x00,        //     XSTART = 0
        0x00, 0x4F,        //     XEND = 79
        RASET, 4,          //  2: Row addr set, 4 args, no delay:
        0x00, 0x00,        //     XSTART = 0
        0x00, 0x9F,        //     XEND = 159
        INVON, 0},         //  3: Invert display  

    Rcmd3[] = {                                                              // 7735R init, part 3 (red or green tab)
        4,                                                                   //  4 commands in list:
        GMCTRP1, 16,                                                         //  1: Gamma Adjustments (pos. polarity), 16 args + delay:
        0x02, 0x1c, 0x07, 0x12,                                              //     (Not entirely necessary, but provides
        0x37, 0x32, 0x29, 0x2d,                                              //      accurate colors)
        0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10, GMCTRN1, 16,         //  2: Gamma Adjustments (neg. polarity), 16 args + delay:
        0x03, 0x1d, 0x07, 0x06,                                              //     (Not entirely necessary, but provides
        0x2E, 0x2C, 0x29, 0x2D,                                              //      accurate colors)
        0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10, NORON, ST_CMD_DELAY, //  3: Normal display on, no args, w/delay
        10,                                                                  //     10 ms delay
        DISPON, ST_CMD_DELAY,                                                //  4: Main screen turn on, no args w/delay
        100};                                                                //    10 ms delay

void lcd_send_command_list(spi_lcd *lcd, uint8_t *addr);
void lcd_send_command(spi_lcd *lcd, uint8_t cmd, uint8_t *data_ptr, uint16_t cnt);

void lcd_st7735_rotate(spi_lcd *lcd, uint8_t m)
{
    uint8_t madctl = 0;

    lcd->rotation = m & 3; // can't be higher than 3

    // For ST7735 with GREEN TAB (including //  HALloWing)...
    if ((lcd->tabcolor == INITR_144GREENTAB) || (lcd->tabcolor == INITR_HALLOWING))
    {
        // ...lcd->_rowstart is 3 for rotations 0&1, 1 for rotations 2&3
        lcd->_rowstart = (lcd->rotation < 2) ? 3 : 1;
    }

    switch (lcd->rotation)
    {
    case 0:
        if (lcd->tabcolor == INITR_BLACKTAB)
        {
            madctl = MADCTL_MX | MADCTL_MY | MADCTL_RGB;
        }
        else
        {
            madctl = MADCTL_MX | MADCTL_MY | MADCTL_BGR;
        }

        if (lcd->tabcolor == INITR_144GREENTAB)
        {
            lcd->height = 128;
            lcd->width = 128;
        }
        else if (lcd->tabcolor == INITR_MINI160x80)
        {
            lcd->height = 160;
            lcd->width = 80;
        }
        else
        {
            lcd->height = 160;
            lcd->width = 128;
        }
        lcd->_xstart = lcd->_colstart;
        lcd->_ystart = lcd->_rowstart;
        break;
    case 1:
        if (lcd->tabcolor == INITR_BLACKTAB)
        {
            madctl = MADCTL_MY | MADCTL_MV | MADCTL_RGB;
        }
        else
        {
            madctl = MADCTL_MY | MADCTL_MV | MADCTL_BGR;
        }

        if (lcd->tabcolor == INITR_144GREENTAB)
        {
            lcd->width = 128;
            lcd->height = 128;
        }
        else if (lcd->tabcolor == INITR_MINI160x80)
        {
            lcd->width = 160;
            lcd->height = 80;
        }
        else
        {
            lcd->width = 160;
            lcd->height = 128;
        }
        lcd->_ystart = lcd->_colstart;
        lcd->_xstart = lcd->_rowstart;
        break;
    case 2:
        if (lcd->tabcolor == INITR_BLACKTAB)
        {
            madctl = MADCTL_RGB;
        }
        else
        {
            madctl = MADCTL_BGR;
        }

        if (lcd->tabcolor == INITR_144GREENTAB)
        {
            lcd->height = 128;
            lcd->width = 128;
        }
        else if (lcd->tabcolor == INITR_MINI160x80)
        {
            lcd->height = 160;
            lcd->width = 80;
        }
        else
        {
            lcd->height = 160;
            lcd->width = 128;
        }
        lcd->_xstart = lcd->_colstart;
        lcd->_ystart = lcd->_rowstart;
        break;
    case 3:
        if (lcd->tabcolor == INITR_BLACKTAB)
        {
            madctl = MADCTL_MX | MADCTL_MV | MADCTL_RGB;
        }
        else
        {
            madctl = MADCTL_MX | MADCTL_MV | MADCTL_BGR;
        }

        if (lcd->tabcolor == INITR_144GREENTAB)
        {
            lcd->width = 128;
            lcd->height = 128;
        }
        else if (lcd->tabcolor == INITR_MINI160x80)
        {
            lcd->width = 160;
            lcd->height = 80;
        }
        else
        {
            lcd->width = 160;
            lcd->height = 128;
        }
        lcd->_ystart = lcd->_colstart;
        lcd->_xstart = lcd->_rowstart;
        break;
    }
    lcd_send_command(lcd, MADCTL, &madctl, 1);
}

void lcd_reset(spi_lcd *lcd);

void lcd_st7735_init(spi_lcd *lcd, uint8_t options)
{

    lcd->rotate_fun = lcd_st7735_rotate;

    lcd_select(lcd);
    delay_ms(1);
    lcd_reset(lcd);
    delay_ms(1);
    lcd_deselect(lcd);
    delay_ms(1);

    lcd_send_command_list(lcd, Rcmd1);
    if (options == INITR_GREENTAB)
    {
        lcd_send_command_list(lcd, Rcmd2green);
        lcd->_colstart = 2;
        lcd->_rowstart = 1;
    }
    else if ((options == INITR_144GREENTAB) || (options == INITR_HALLOWING))
    {
        lcd->height = 128;
        lcd->width = 128;
        lcd_send_command_list(lcd, Rcmd2green144);
        lcd->_colstart = 2;
        lcd->_rowstart = 3; // For default rotation 0
    }
    else if (options == INITR_MINI160x80)
    {
        lcd->height = 80;
        lcd->width = 160;
        lcd_send_command_list(lcd, Rcmd2green160x80);
        lcd->_colstart = 26;
        lcd->_rowstart = 1;
    }
    else
    {
        // colstart, rowstart left at default '0' values
        lcd_send_command_list(lcd, Rcmd2red);
    }
    lcd_send_command_list(lcd, Rcmd3);

    // Black tab, change MADCTL color filter
    if ((options == INITR_BLACKTAB) || (options == INITR_MINI160x80))
    {
        uint8_t data = 0xC0;
        lcd_send_command(lcd, MADCTL, &data, 1);
    }

    if (options == INITR_HALLOWING)
    {
        // //  HALlowing is simply a 1.44" green tab upside-down:
        lcd->tabcolor = INITR_144GREENTAB;
        lcd_st7735_rotate(lcd, 2);
    }
    else
    {
        lcd->tabcolor = options;
        lcd_st7735_rotate(lcd, 0);
    }

    lcd_st7735_rotate(lcd, 2);
}
