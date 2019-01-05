/*
 * ILI9341_TFT_LCD_ATMEGA32.c
 *
 * Created: 2018-11-06 22:21:06
 * Author : Daniel
 * Program do obslugi wyswietlacza LCD TFT na ukladzie ILI9341 przez interfejs SPI.
 * 2.4" LCD TFT module.
 * wersja 1.0 - bedzie rozbudowywana o kolejne funkcjonalnosci - digitizer, obsluga karty SD, nowe fonty.
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "ILI9341.h"
#include "colors.h"
#include "touch.h"
#include "font16x16.h"



int main(void)
{
    ILI9341_init();
	ILI9341_set_rotation(LANDSCAPE);
	ILI9341_cls(BLUE);	// czysci ekran na niebiesko
	ILI9341_set_font((font_t) {font16x16, 16, 16, YELLOW, BLUE});	// ale tu pewnie cos trzeba zaimportowac jeszcze
	ILI9341_txt(1, 1, "Daniel");
	_delay_ms(1000);
	for (uint8_t i = 0; i<100; i+= 3)
	{
		ILI9341_draw_pixel(i, 100, YELLOW);	// linia przerywana - co 3 pixele
	}
	_delay_ms(1000);
	
    while (1) 
    {
		
    }
}

