#include <stdint.h>
#include "spi_lcd.h"
#include "mipi_reg.h"

#define lcd_set_gpio( pin, val ) (*(((lcd)->set_gpio)))( pin, val ) // 1

#define lcd_tx_8b(spi_hw, ptr, cnt) (*(((lcd)->spidev)->write_8b))(spi_hw, ptr, cnt)
#define lcd_tx_16b(spi_hw, ptr, cnt) (*(((lcd)->spidev)->write_16b))(spi_hw, ptr, cnt)

#define delay_ms(ms) (*(((lcd)->spidev)->delay_ms))(ms)
// #define delay_us(us) (*(((lcd)->spidev).delay_us))(us)

void lcd_select(spi_lcd *lcd)
{
    lcd_set_gpio(lcd->pin_cs, 0);
}

void lcd_deselect(spi_lcd *lcd)
{
    lcd_set_gpio(lcd->pin_cs, 1);
}

void lcd_reset(spi_lcd *lcd)
{
    lcd_set_gpio(lcd->pin_rst, 0);
    delay_ms(5);
    lcd_set_gpio(lcd->pin_rst, 1);
}

void lcd_write_command(spi_lcd *lcd, uint8_t cmd)
{
    lcd_set_gpio( lcd->pin_dc, 0);
    lcd_tx_8b(lcd->spidev->hw_spi, &cmd, 1);
}

void lcd_write_data(spi_lcd *lcd, uint8_t *ptr, uint16_t cnt)
{
    lcd_set_gpio( lcd->pin_dc, 1);
    lcd_tx_8b(lcd->spidev->hw_spi, ptr, cnt);
}

void lcd_send_command(spi_lcd *lcd, uint8_t cmd, uint8_t *data_ptr, uint16_t cnt)
{
    lcd_select(lcd);
    lcd_write_command(lcd, cmd);
    if (cnt)
        lcd_write_data(lcd, data_ptr, cnt);
    lcd_deselect(lcd);
}

void lcd_send_command_list(spi_lcd *lcd, uint8_t *addr)
{
    uint8_t numCommands, cmd, numArgs;
    uint16_t ms;

    numCommands = *(addr++); // Number of commands to follow
    while (numCommands--)
    {                                // For each command...
        cmd = *(addr++);             // Read command
        numArgs = *(addr++);         // Number of args to follow
        ms = numArgs & ST_CMD_DELAY; // If hibit set, delay follows args
        numArgs &= ~ST_CMD_DELAY;    // Mask out delay bit
        lcd_send_command(lcd, cmd, addr, numArgs);
        addr += numArgs;

        if (ms)
        {
            ms = *(addr++); // Read post-command delay time (ms)
            if (ms == 255)
                ms = 500; // If 255, delay for 500 ms
            delay_ms(ms);
        }
    }
}

void lcd_set_window(spi_lcd *lcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    x += lcd->_xstart;
    y += lcd->_ystart;

    uint32_t xa = ((uint32_t)x << 16) | (x + w - 1);
    uint32_t ya = ((uint32_t)y << 16) | (y + h - 1);

    xa = __builtin_bswap32(xa);
    ya = __builtin_bswap32(ya);

    lcd_send_command(lcd, CASET, &xa, sizeof(xa));
    lcd_send_command(lcd, RASET, &ya, sizeof(ya));
    lcd_send_command(lcd, RAMWR, 0, 0);
}

// Graphics Functions
void lcd_flush(spi_lcd *lcd)
{
    (void)lcd;
}

void lcd_fill_rect(spi_lcd *lcd, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    lcd_set_window(lcd, x, y, w, h);
    uint32_t count = w * h;

    lcd_select(lcd);
    lcd_set_gpio( lcd->pin_dc, 1);
    for (int i = 0; i < count; i++)
        lcd_tx_16b(lcd->spidev->hw_spi, &color, 1);
    lcd_deselect(lcd);
}

void lcd_fill(spi_lcd *lcd, uint16_t color)
{
    lcd_fill_rect(lcd, 0, 0, lcd->width, lcd->height, color);
}

void lcd_clear(spi_lcd *lcd)
{
    lcd_fill(lcd, RGB565_BLACK);
}

void lcd_draw_pixel(spi_lcd *lcd, int16_t x, int16_t y, uint16_t color)
{
    lcd_set_window(lcd, x, y, 1, 1);

    lcd_select(lcd);
    lcd_set_gpio( lcd->pin_dc, 1);
    lcd_tx_16b(lcd->spidev->hw_spi, &color, 1);
    lcd_deselect(lcd);
}

void lcd_draw_bitmap(spi_lcd *lcd, int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h)
{
    lcd_set_window(lcd, x, y, w, h);

    lcd_select(lcd);
    lcd_set_gpio( lcd->pin_dc, 1);
    lcd_tx_16b(lcd->spidev->hw_spi, bitmap, w * h);
    lcd_deselect(lcd);
}

void lcd_rotate_internal(spi_lcd *lcd, uint8_t m)
{
    (void)lcd;
}