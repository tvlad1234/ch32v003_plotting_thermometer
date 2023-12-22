#include "ch32v003fun.h"
#include <stdio.h>

#include "bmp085.h"
#include "gfx.h"
#include "plot.h"

#include "spi_lcd.h"
#include "ssd1306.h"
#include "st7735.h"


#define I2C_GPIO GPIOD
#define I2C_SDA_PIN 5
#define I2C_SCL_PIN 6

#define SPI_GPIO GPIOC
#define SPI_TX_PIN 4
#define SPI_CLK_PIN 3


#define LCD_GPIO GPIOC
#define LCD_PIN_CS 1
#define LCD_PIN_DC 2
#define LCD_PIN_RST 6

#include "ch32v003_I2C_bitbang.h"
#include "ch32v003_SPI_bitbang.h"

void gpio_mode_output( GPIO_TypeDef *gpio, uint8_t pin )
{
	gpio->CFGLR &= ~( 0xf << ( 4 * pin ) );
	gpio->CFGLR |= ( GPIO_Speed_10MHz | GPIO_CNF_OUT_PP ) << ( 4 * pin );
}

// glue functions

void oled_link_gfx( ssd1306_oled *oled, gfx_inst *gfx )
{
	gfx->disp_ptr = oled;
	gfx->height = oled->height;
	gfx->width = oled->width;
	gfx->pixel_draw_fun = oled_draw_pixel;
	gfx->flush_fun = oled_flush;

	gfx_set_text_color( gfx, WHITE, BLACK );
	gfx_set_text_size( gfx, 1 );
	gfx_set_cursor( gfx, 0, 0 );
}

void spi_lcd_link_gfx( spi_lcd *lcd, gfx_inst *gfx )
{
	gfx->disp_ptr = lcd;
	gfx->height = lcd->height;
	gfx->width = lcd->width;
	gfx->pixel_draw_fun = lcd_draw_pixel;
	gfx->fill_rect_fun = lcd_fill_rect;
	gfx->flush_fun = lcd_flush;
	gfx->rgb565_bmp_fun = lcd_draw_bitmap;

	gfx_set_text_color( gfx, RGB565_WHITE, BLACK );
	gfx_set_text_size( gfx, 1 );
	gfx_set_cursor( gfx, 0, 0 );
}

void glue_i2c_bitbang_read_mem( void *dev, uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t count )
{
	i2c_read_mem( addr, reg, buf, count );
}

void glue_i2c_bitbang_write_mem( void *dev, uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t count )
{
	i2c_write_mem( addr, reg, buf, count );
}

void glue_spi_bitbang_write_8b( void *spidev_hw, uint8_t *ptr, uint32_t cnt )
{
	bitbang_spi_write_8b( ptr, cnt );
}

void glue_spi_bitbang_write_16b( void *spidev_hw, uint16_t *ptr, uint32_t cnt )
{
	bitbang_spi_write_16b( ptr, cnt );
}

void spi_lcd_set_gpio( uint16_t pin, uint8_t s )
{
	if ( s )
		LCD_GPIO->BSHR = ( 1 << pin );
	else
		LCD_GPIO->BSHR = ( 1 << ( 16 + pin ) );
}

void glue_delay_ms( uint16_t ms )
{
	Delay_Ms( ms );
}

void i2cdev_init_bitbang( i2cdev_t *i2cdev )
{
	i2c_gpio_init();
	i2cdev->tx_mem = glue_i2c_bitbang_write_mem;
	i2cdev->rx_mem = glue_i2c_bitbang_read_mem;
	i2cdev->delay_ms = glue_delay_ms;
	i2cdev->hw_i2c = NULL;
}

void spidev_init_bitbang( spidev_t *spidev )
{
	spi_gpio_init();
	spidev->write_8b = glue_spi_bitbang_write_8b;
	spidev->write_16b = glue_spi_bitbang_write_16b;
	spidev->delay_ms = glue_delay_ms;
	spidev->hw_spi = NULL;
}

// global vars

i2cdev_t myI2c;
spidev_t mySpi;

// ssd1306_oled myOled;
spi_lcd myLcd;

gfx_inst myGfx;
bmp085_t myBmp;

plot_t tempPlot;
plot_t presPlot;

float tempValues[40];
float presValues[40];

int main()
{
	SystemInit();

	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC;

	i2cdev_init_bitbang( &myI2c );
	spidev_init_bitbang( &mySpi );

	gpio_mode_output( LCD_GPIO, LCD_PIN_CS );
	gpio_mode_output( LCD_GPIO, LCD_PIN_DC );
	gpio_mode_output( LCD_GPIO, LCD_PIN_RST );

	myLcd.pin_cs = LCD_PIN_CS;
	myLcd.pin_dc = LCD_PIN_DC;
	myLcd.pin_rst = LCD_PIN_RST;

	myLcd.spidev = &mySpi;
	myLcd.set_gpio = spi_lcd_set_gpio;

	lcd_st7735_init( &myLcd, INITR_MINI160x80 );
	lcd_rotate( &myLcd, 3 );
	spi_lcd_link_gfx( &myLcd, &myGfx );

	// oled_init( &myOled, &myI2c, 0x3C, SSD1306_SWITCHCAPVCC, 128, 64 );
	// oled_link_gfx( &myOled, &myGfx );

	tempPlot.color_bg = BLACK;
	tempPlot.color_frame = RGB565_WHITE;
	tempPlot.color_plot = RGB565_GREEN;

	tempPlot.pos_x = 0;
	tempPlot.pos_y = 16;

	tempPlot.width = myGfx.width;
	tempPlot.height = myGfx.height - tempPlot.pos_y;

	tempPlot.val_min = 21;
	tempPlot.val_max = 21;

	tempPlot.point_dist = 10;

	tempPlot.autorange = 1;
	tempPlot.circle_points = 1;
	tempPlot.circle_radius = 2;
	tempPlot.circle_color = RGB565_RED;

	tempPlot.points = tempValues;
	tempPlot.point_num = 0;

	presPlot.color_bg = BLACK;
	presPlot.color_frame = RGB565_WHITE;
	presPlot.color_plot = RGB565_BLUE;

	presPlot.pos_x = 0;
	presPlot.pos_y = 16;

	presPlot.width = myGfx.width;
	presPlot.height = myGfx.height - presPlot.pos_y;

	presPlot.val_min = 1000;
	presPlot.val_max = 1000;

	presPlot.point_dist = 10;

	presPlot.autorange = 1;
	presPlot.circle_points = 1;
	presPlot.circle_radius = 2;
	presPlot.circle_color = RGB565_RED;

	presPlot.points = presValues;
	presPlot.point_num = 0;

	plot_render( &tempPlot, &myGfx );
	plot_render( &presPlot, &myGfx );
	gfx_clear( &myGfx );

	if ( bmp085_init( &myBmp, &myI2c, BMP085_ULTRAHIGHRES ) )
	{
		gfx_clear( &myGfx );
		gfx_set_cursor( &myGfx, 0, 0 );
		gfx_print_string( &myGfx, "sensor not found\n" );
		gfx_flush( &myGfx );
		while ( 1 )
			;
	}

	int c = 0;
	uint8_t sel;
	float lastTemp;
	int32_t lastPres;

	while ( 1 )
	{
		float temp = 0;
		int32_t pres = 0;

		for ( int i = 0; i < 30; i++ )
		{

			sel = !sel;
			gfx_clear( &myGfx );
			gfx_set_cursor( &myGfx, 48, 4 );

			if ( c )
			{
				if ( sel )
				{
					int intTemp = lastTemp;
					int decTemp = ( (int)( lastTemp * 10 ) % 10 );
					gfx_printf( &myGfx, "%d.%d deg C", intTemp, decTemp );
					plot_render( &tempPlot, &myGfx );
				}
				else
				{
					gfx_printf( &myGfx, "%d hPa", lastPres );
					plot_render( &presPlot, &myGfx );
				}
			}
			else
				gfx_printf( &myGfx, "Measuring.." );

			gfx_set_cursor( &myGfx, 0, 0 );
			gfx_printf( &myGfx, "%d", i + 1 );

			gfx_flush( &myGfx );

			temp += bmp085_temperature( &myBmp );
			pres += bmp085_pressure( &myBmp ) / 100;

			Delay_Ms( 4000 );
		}

		temp /= 30.0f;
		pres /= 30.0f;

		plot_add_point( &presPlot, pres );
		plot_add_point( &tempPlot, temp );

		lastTemp = temp;
		lastPres = pres;

		c = 1;
	}
}
