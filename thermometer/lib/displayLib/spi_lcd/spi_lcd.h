#ifndef _SPI_LCD_H
#define _SPI_LCD_H

#include <stdint.h>
#include "spidev.h"

#define RGB565_BLACK 0x0000
#define RGB565_WHITE 0xFFFF
#define RGB565_RED 0xF800
#define RGB565_GREEN 0x07E0
#define RGB565_BLUE 0x001F
#define RGB565_CYAN 0x07FF
#define RGB565_MAGENTA 0xF81F
#define RGB565_YELLOW 0xFFE0
#define RGB565_ORANGE 0xFC00

typedef struct spi_lcd spi_lcd;

struct spi_lcd
{
	uint16_t pin_cs, pin_dc, pin_rst;
	spidev_t *spidev;

	uint16_t width, height, rotation;
	uint16_t window_width, window_height;
	uint16_t _colstart, _rowstart, _colstart2, _rowstart2;
	int16_t _xstart;
	int16_t _ystart;

	uint8_t tabcolor;
	void ( *rotate_fun )( spi_lcd *, uint8_t );

    void ( *set_gpio )( uint16_t, uint8_t );

};


#define lcd_rotate( lcd_ptr, r ) ( *( ( lcd_ptr )->rotate_fun ) )( lcd_ptr, r )

void lcd_flush( spi_lcd *lcd );
void lcd_clear( spi_lcd *lcd );

void lcd_fill_rect( spi_lcd *lcd, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color );
void lcd_fill( spi_lcd *lcd, uint16_t color );

void lcd_draw_pixel( spi_lcd *lcd, int16_t x, int16_t y, uint16_t color );
void lcd_draw_bitmap( spi_lcd *lcd, int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h );

#endif