/*
 * SPI.c
 *
 *  
 */

#include <avr/io.h>
#include "SPI.h"
#include "ILI9341.h"
#include "touch.h"

void SPI_init(void)
{
       SPCR = ((1 << SPE) |													// SPI Enable
            (0 << SPIE) |													// SPI Interupt not used
            (0 << DORD) |													// Data Order (0:MSB first / 1:LSB first)
            (1 << MSTR) |													// Master/Slave select
            (0 << SPR1) | (0 << SPR0) |										// SPI Clock Rate F_CPU/2
            (0 << CPOL) |													// Clock Polarity (0:SCK low / 1:SCK hi when idle)
            (0 << CPHA));													// Clock Phase (0:leading / 1:trailing edge sampling)
    SPSR = (1 << SPI2X);													// Double Clock Rate F_CPU/2
}

/* Exchange byte via SPI */
uint8_t SPI_rxtx(uint8_t tx, enum dev_t device)
{
#ifdef USE_HARD_SPI
	SPDR = tx;																// Dummy byte

    while (!(SPSR & (1 << SPIF)));											// Wait for transmission complete

#else
	uint8_t rx;
    for (uint8_t i = 0x80; i; i >>= 1)										// 8 bits (from MSB)
    {
        switch (device)
        {
            case TFT:
				TFT_SCK_LO;													// Clock LOW
				TFT_SCK_HI;													// Clock HIGH
				rx <<= 1;													// Shift bit

				if (TFT_MISO_PIN & TFT_MISO) rx |= 0x01;					// If MISO set, then set bit
                break;

            case TOUCH:
				TOUCH_SCK_LO;												// Clock LOW
				TOUCH_SCK_HI;												// Clock HIGH
				rx <<= 1;													// Shift bit

				if (TOUCH_MISO_PIN & TOUCH_MISO) rx |= 0x01;				// If MISO set, then set bit
                break;

            case SDCARD:
				//SD_SCK_LO;													// Clock LOW
				//SD_SCK_HI;													// Clock HIGH
				//rx <<= 1;													// Shift bit
//
				//if (SD_MISO_PIN & SD_MISO) rx |= 0x01;						// If MISO set, then set bit
                break;
        }
    }

#endif
    return SPDR;
}
