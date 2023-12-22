#ifndef _SPIDEV_H
#define _SPIDEV_H

struct spidev_t
{
	void *hw_spi;
	void ( *write_8b )( void *, uint8_t *, uint32_t );
	void ( *write_16b )( void *, uint16_t *, uint32_t );
	
	void ( *delay_ms )( uint16_t );
};
typedef struct spidev_t spidev_t;

#endif
