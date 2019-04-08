/*
 * sd_card.h
 *
 *  Daniel
 */ 
 
#ifndef SD_CARD_H_
#define SD_CARD_H_

#define	USE_SDCARD_CS	1

// definicje pinow
#define		SD_SCK	(1<<7)													// CLK
#define 	SD_MISO	(1<<6)													// MISO
#define 	SD_MOSI	(1<<5)													// MOSI
#define		SD_CS	(1<<0)													// CS

// definicje portow
#define 	SD_SCK_PORT		PORTB
#define 	SD_SCK_DIR		DDRB
#define 	SD_MOSI_PORT	PORTB
#define 	SD_MOSI_DIR		DDRB
#define 	SD_MISO_PORT	PORTB
#define 	SD_MISO_DIR		DDRB
#define 	SD_MISO_PIN		PINB
#define		SD_CS_PORT		PORTD
#define		SD_CS_DIR		DDRD
#define		SD_CS_PIN		PIND

// definicje makr
#define 	SD_SCK_LO	SD_SCK_PORT &= ~SD_SCK
#define 	SD_SCK_HI	SD_SCK_PORT |= SD_SCK
#define 	SD_MOSI_LO	SD_MOSI_PORT &= ~SD_MOSI
#define 	SD_MOSI_HI	SD_MOSI_PORT |= SD_MOSI
#define		SD_CS_LO	SD_CS_PORT &= ~SD_CS								// CS=low
#define		SD_CS_HI	SD_CS_PORT |= SD_CS									// CS=high
#define		SD_CS_X		SD_CS_PIN & SD_CS									// CS state

void SDCARD_init_io(void);

#endif /* SD_CARD_H_ */