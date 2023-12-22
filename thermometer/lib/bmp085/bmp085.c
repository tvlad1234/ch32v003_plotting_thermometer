#include <stddef.h>
#include <stdint.h>

#include "bmp085.h"

// Calibration data registers
#define BMP085_CAL_AC1 0xAA //!< R   Calibration data (16 bits)
#define BMP085_CAL_AC2 0xAC //!< R   Calibration data (16 bits)
#define BMP085_CAL_AC3 0xAE //!< R   Calibration data (16 bits)
#define BMP085_CAL_AC4 0xB0 //!< R   Calibration data (16 bits)
#define BMP085_CAL_AC5 0xB2 //!< R   Calibration data (16 bits)
#define BMP085_CAL_AC6 0xB4 //!< R   Calibration data (16 bits)
#define BMP085_CAL_B1 0xB6 //!< R   Calibration data (16 bits)
#define BMP085_CAL_B2 0xB8 //!< R   Calibration data (16 bits)
#define BMP085_CAL_MB 0xBA //!< R   Calibration data (16 bits)
#define BMP085_CAL_MC 0xBC //!< R   Calibration data (16 bits)
#define BMP085_CAL_MD 0xBE //!< R   Calibration data (16 bits)

// Commands
#define BMP085_CONTROL 0xF4 //!< Control register
#define BMP085_TEMPDATA 0xF6 //!< Temperature data register
#define BMP085_PRESSUREDATA 0xF6 //!< Pressure data register
#define BMP085_READTEMPCMD 0x2E //!< Read temperature control register value
#define BMP085_READPRESSURECMD 0x34 //!< Read pressure control register value

// I2C handling functions
uint8_t bmpRead8( bmp085_t *sensor, uint8_t a )
{
	uint8_t r;
	sensor->i2cdev->rx_mem( NULL, sensor->addr, a, &r, 1 );
	return r;
}

uint16_t bmpRead16( bmp085_t *sensor, uint8_t a )
{
	uint8_t retbuf[2];
	uint16_t r;
	sensor->i2cdev->rx_mem( NULL, sensor->addr, a, retbuf, 2 );
	r = retbuf[1] | ( retbuf[0] << 8 );
	return r;
}

void bmpWrite8( bmp085_t *sensor, uint8_t a, uint8_t d )
{
	sensor->i2cdev->tx_mem( NULL, sensor->addr, a, &d, 1 );
}

uint8_t bmp085_init( bmp085_t *sensor, i2cdev_t *i2cdev, uint8_t mode )
{
	sensor->i2cdev = i2cdev;
	sensor->addr = 0x77;

	if ( mode > BMP085_ULTRAHIGHRES ) mode = BMP085_ULTRAHIGHRES;
	sensor->oversampling = mode;

	if ( bmpRead8( sensor, 0xD0 ) != 0x55 ) return 1;

	/* read calibration data */
	sensor->ac1 = bmpRead16( sensor, BMP085_CAL_AC1 );
	sensor->ac2 = bmpRead16( sensor, BMP085_CAL_AC2 );
	sensor->ac3 = bmpRead16( sensor, BMP085_CAL_AC3 );
	sensor->ac4 = bmpRead16( sensor, BMP085_CAL_AC4 );
	sensor->ac5 = bmpRead16( sensor, BMP085_CAL_AC5 );
	sensor->ac6 = bmpRead16( sensor, BMP085_CAL_AC6 );

	sensor->b1 = bmpRead16( sensor, BMP085_CAL_B1 );
	sensor->b2 = bmpRead16( sensor, BMP085_CAL_B2 );

	sensor->mb = bmpRead16( sensor, BMP085_CAL_MB );
	sensor->mc = bmpRead16( sensor, BMP085_CAL_MC );
	sensor->md = bmpRead16( sensor, BMP085_CAL_MD );

	return 0;
}

// Sensor read functions
int32_t computeB5( bmp085_t *sensor, int32_t UT )
{
	int32_t X1 = ( UT - (int32_t)( sensor->ac6 ) ) * ( (int32_t)( sensor->ac5 ) ) >> 15;
	int32_t X2 = ( (int32_t)( sensor->mc ) << 11 ) / ( X1 + (int32_t)( sensor->md ) );
	return X1 + X2;
}

uint16_t readBMPRawTemperature( bmp085_t *sensor )
{
	bmpWrite8( sensor, BMP085_CONTROL, BMP085_READTEMPCMD );
	sensor->i2cdev->delay_ms( 5 );
	return bmpRead16( sensor, BMP085_TEMPDATA );
}

uint32_t readBMPRawPressure( bmp085_t *sensor )
{
	uint32_t raw;

	bmpWrite8( sensor, BMP085_CONTROL, BMP085_READPRESSURECMD + ( sensor->oversampling << 6 ) );

	if ( sensor->oversampling == BMP085_ULTRALOWPOWER )
		sensor->i2cdev->delay_ms( 5 );
	else if ( sensor->oversampling == BMP085_STANDARD )
		sensor->i2cdev->delay_ms( 8 );
	else if ( sensor->oversampling == BMP085_HIGHRES )
		sensor->i2cdev->delay_ms( 14 );
	else
		sensor->i2cdev->delay_ms( 26 );

	raw = bmpRead16( sensor, BMP085_PRESSUREDATA );

	raw <<= 8;
	raw |= bmpRead8( sensor, BMP085_PRESSUREDATA + 2 );
	raw >>= ( 8 - sensor->oversampling );

	return raw;
}

float bmp085_temperature( bmp085_t *sensor )
{
	int32_t UT, B5; // following ds convention
	float temp;

	UT = readBMPRawTemperature( sensor );

	B5 = computeB5( sensor, UT );
	temp = ( B5 + 8 ) >> 4;
	temp /= 10;

	return temp;
}

int32_t bmp085_pressure( bmp085_t *sensor )
{
	int32_t UT, UP, B3, B5, B6, X1, X2, X3, p;
	uint32_t B4, B7;

	UT = readBMPRawTemperature( sensor );
	UP = readBMPRawPressure( sensor );

	B5 = computeB5( sensor, UT );

	// do pressure calcs
	B6 = B5 - 4000;
	X1 = ( (int32_t)sensor->b2 * ( ( B6 * B6 ) >> 12 ) ) >> 11;
	X2 = ( (int32_t)sensor->ac2 * B6 ) >> 11;
	X3 = X1 + X2;
	B3 = ( ( ( (int32_t)sensor->ac1 * 4 + X3 ) << sensor->oversampling ) + 2 ) / 4;

	X1 = ( (int32_t)sensor->ac3 * B6 ) >> 13;
	X2 = ( (int32_t)sensor->b1 * ( ( B6 * B6 ) >> 12 ) ) >> 16;
	X3 = ( ( X1 + X2 ) + 2 ) >> 2;
	B4 = ( (uint32_t)sensor->ac4 * (uint32_t)( X3 + 32768 ) ) >> 15;
	B7 = ( (uint32_t)UP - B3 ) * (uint32_t)( 50000UL >> sensor->oversampling );

	if ( B7 < 0x80000000 )
	{
		p = ( B7 * 2 ) / B4;
	}
	else
	{
		p = ( B7 / B4 ) * 2;
	}
	X1 = ( p >> 8 ) * ( p >> 8 );
	X1 = ( X1 * 3038 ) >> 16;
	X2 = ( -7357 * p ) >> 16;

	p = p + ( ( X1 + X2 + (int32_t)3791 ) >> 4 );

	return p;
}
