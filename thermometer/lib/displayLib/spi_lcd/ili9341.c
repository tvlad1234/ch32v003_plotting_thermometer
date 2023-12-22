#include "spi_lcd.h"
#include "mipi_reg.h"

#include "ili9341.h"

#define TFTWIDTH 240  ///< ILI9341 max TFT width
#define TFTHEIGHT 320 ///< ILI9341 max TFT height

#define delay_ms(ms) (*(((lcd)->spidev)->delay_ms))(ms)
void lcd_select(spi_lcd *lcd);
void lcd_deselect(spi_lcd *lcd);

void lcd_send_command(spi_lcd *lcd, uint8_t cmd, uint8_t *data_ptr, uint16_t cnt);
void lcd_send_command_list(spi_lcd *lcd, uint8_t *addr);
void lcd_reset(spi_lcd *lcd);

const uint8_t initcmd[] = {
    24, // 24 commands
    0xEF, 3, 0x03, 0x80, 0x02,
    0xCF, 3, 0x00, 0xC1, 0x30,
    0xED, 4, 0x64, 0x03, 0x12, 0x81,
    0xE8, 3, 0x85, 0x00, 0x78,
    0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
    0xF7, 1, 0x20,
    0xEA, 2, 0x00, 0x00,
    PWCTR1, 1, 0x23,       // Power control VRH[5:0]
    PWCTR2, 1, 0x10,       // Power control SAP[2:0];BT[3:0]
    VMCTR1, 2, 0x3e, 0x28, // VCM control
    VMCTR2, 1, 0x86,       // VCM control2
    MADCTL, 1, 0x48,       // Memory Access Control
    VSCRSADD, 1, 0x00,     // Vertical scroll zero
    COLMOD, 1, 0x55,
    FRMCTR1, 2, 0x00, 0x18,
    DISSET5, 3, 0x08, 0x82, 0x27,                    // Display Function Control
    0xF2, 1, 0x00,                                   // 3Gamma Function Disable
    GAMMASET, 1, 0x01,                               // Gamma curve selected
    GMCTRP1, 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, // Set Gamma
    0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
    GMCTRN1, 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, // Set Gamma
    0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
    SLPOUT, ST_CMD_DELAY, 150, // Exit Sleep
    DISPON, ST_CMD_DELAY, 150, // Display on
    0x00                       // End of list
};

void lcd_ili9341_rotate(spi_lcd *lcd, uint8_t m)
{
    lcd->rotation = m % 4; // can't be higher than 3
    switch (lcd->rotation)
    {
    case 0:
        m = (MADCTL_MX | MADCTL_BGR);
        lcd->width = TFTWIDTH;
        lcd->height = TFTHEIGHT;
        break;
    case 1:
        m = (MADCTL_MV | MADCTL_BGR);
        lcd->width = TFTHEIGHT;
        lcd->height = TFTWIDTH;
        break;
    case 2:
        m = (MADCTL_MY | MADCTL_BGR);
        lcd->width = TFTWIDTH;
        lcd->height = TFTHEIGHT;
        break;
    case 3:
        m = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
        lcd->width = TFTHEIGHT;
        lcd->height = TFTWIDTH;
        break;
    }

    lcd_send_command(lcd, MADCTL, &m, 1);
}

void lcd_ili9341_init(spi_lcd *lcd)
{
    lcd->rotate_fun = lcd_ili9341_rotate;

    lcd_select(lcd);
    delay_ms(1);
    lcd_reset(lcd);
    delay_ms(150);
    lcd_deselect(lcd);
    delay_ms(1);

    lcd_send_command_list(lcd, initcmd);

    lcd->width = TFTWIDTH;
    lcd->height = TFTHEIGHT;
}
