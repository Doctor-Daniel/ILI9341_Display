/*
 * SPI.c
 *
 *  
 */

#include <avr/io.h>
#include "SPI.h"
#include "ILI9341.h"
#include "touch.h"

//#define USE_HARD_SPI

void SPI_init(void)
{
       SPCR = (_BV(SPE) | _BV(MSTR));		// SPI enabled. Master/Slave select. MSB first. SPI Clock rate F_CPU / 2. SCK low. Clock phase leading.
	   SPSR = _BV(SPI2X);					// F_CPU/2 enabled
}

/* Exchange byte via SPI */
uint8_t SPI_rxtx(uint8_t tx, enum dev_t device)								// Wymiana danych przez szyne SPI
{
//#ifdef USE_HARD_SPI
	SPDR = tx;																// Dummy byte

    while (!(SPSR & (1 << SPIF)));											// Oczekiwanie na koniec transmisji
//---------------------------------------------------------------------------------------------------------------------------
//#else
	//uint8_t rx;
    //for (uint8_t i = 0x80; i; i >>= 1)										// 8 bitow od More Significant Byte (MSB)
    //{
        //switch (device)
        //{
            //case TFT:
				//TFT_SCK_LO;													// Pin SCK stan niski
				//TFT_SCK_HI;													// Pin SCK stan wysoki
				//rx <<= 1;													// przesun bit o 1 w prawo
//
				//if (TFT_MISO_PIN & TFT_MISO) rx |= 0x01;					// Jesli MISO ustawione to ustaw bit 0. 
                //break;
//
            //case TOUCH:
				//TOUCH_SCK_LO;												// Clock LOW
				//TOUCH_SCK_HI;												// Clock HIGH
				//rx <<= 1;													// Shift bit
//
				//if (TOUCH_MISO_PIN & TOUCH_MISO) rx |= 0x01;				// If MISO set, then set bit
                //break;
//
            //case SDCARD:
				////SD_SCK_LO;													// Clock LOW
				////SD_SCK_HI;													// Clock HIGH
				////rx <<= 1;													// Shift bit
////
				////if (SD_MISO_PIN & SD_MISO) rx |= 0x01;						// If MISO set, then set bit
                //break;
        //}
    //}
//
//#endif
//----------------------------------------------------------------------------------------------------------------------
    return SPDR;
}
