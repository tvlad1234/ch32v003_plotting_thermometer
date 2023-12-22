#ifndef _I2CDEV_H
#define _I2CDEV_H

struct i2cdev_t
{
	void *hw_i2c;
	void ( *tx_bytes )( void *, uint8_t *, uint16_t );
	void ( *rx_bytes )( void *, uint8_t *, uint16_t );
	void ( *tx_mem )( void *, uint8_t, uint8_t, uint8_t *, uint16_t );
	void ( *rx_mem )( void *, uint8_t, uint8_t, uint8_t *, uint16_t );
	void ( *delay_ms )( uint16_t );
};
typedef struct i2cdev_t i2cdev_t;

#endif
