/*
 * sd_card.c
 *
 *  Author: Daniel
 */ 

 #include <avr/io.h>
 #include "sd_card.h"

 void SDCARD_init_io(void)
 {
	 // This is already done in TFT section
	 SD_MOSI_DIR |= SD_MOSI;										// SD_MOSI pin as output
	 SD_SCK_DIR |= SD_SCK;										// SD_SCK pin as output
	 SD_MOSI_PORT |= SD_MOSI;										// Hi state
	 SD_SCK_PORT |= SD_SCK;
	
	 #if USE_SDCARD_CS == 1			// If TFT_CS in use
	 SD_CS_DIR |= SD_CS;
	 SD_CS_PORT |= SD_CS;
	 #endif
 };
