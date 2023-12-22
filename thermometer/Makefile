all : flash

TARGET:=plotting_thermometer

ADDITIONAL_C_FILES+= lib/displayLib/ssd1306/ssd1306.c lib/displayLib/spi_lcd/spi_lcd.c lib/displayLib/spi_lcd/st7735.c lib/displayLib/gfx/gfx.c lib/plot/plot.c lib/bmp085/bmp085.c

CFLAGS+= \
	-Ilib/displayLib/gfx \
	-Ilib/bmp085 \
	-Ilib/displayLib/spi_lcd \
	-Ilib//displayLib/ssd1306 \
	-Ilib/bus_helper \
	-Ilib/plot

CH32V003FUN=../ch32v003fun/ch32v003fun
include ../ch32v003fun/ch32v003fun/ch32v003fun.mk

# FLASH_COMMAND+= -c COM10

flash : cv_flash
clean : cv_clean


