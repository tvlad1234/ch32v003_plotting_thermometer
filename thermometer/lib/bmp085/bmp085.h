#ifndef _BMP085_H
#define _BMP085_H

#include "i2cdev.h"


struct bmp085_t
{
	i2cdev_t *i2cdev;
	uint8_t addr; // Use 8-bit address
	uint8_t oversampling;

	// Calibration data (will be read from sensor)
	int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
	uint16_t ac4, ac5, ac6;
};

typedef struct bmp085_t bmp085_t;

// Modes
#define BMP085_ULTRALOWPOWER 0 //!< Ultra low power mode
#define BMP085_STANDARD 1 //!< Standard mode
#define BMP085_HIGHRES 2 //!< High-res mode
#define BMP085_ULTRAHIGHRES 3 //!< Ultra high-res mode

#define STD_ATM_PRESS 101325

// Sensor Init function
uint8_t bmp085_init( bmp085_t *sensor, i2cdev_t *i2cdev, uint8_t mode );

float bmp085_temperature( bmp085_t *sensor );
int32_t bmp085_pressure( bmp085_t *sensor );

#endif /* _BMP085_H */
