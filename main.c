/*
 * ILI9341_TFT_LCD_ATMEGA32.c
 *
 * Created: 2018-11-06 22:21:06
 * Author : Daniel
 * Program do obslugi wyswietlacza LCD TFT na ukladzie ILI9341 przez interfejs SPI.
 * 2.4" LCD TFT module.
 * wersja 1.1 - obsluga wyswietlacza ILI9341, digitizera XPT2046. Brak jeszcze obslugi SD.
 * ---------------------------------------------------------------------------------------
 * Do mikrokontrolera musi byc wgrany wsad FLASH i wsad EEPROM. Inaczej nie dziala digitizer.
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "ILI9341.h"
#include "colors.h"
#include "touch.h"
#include "font16x16.h"
#include "font5x7.h"
#include "font8x8.h"
#include "font8x12.h"
#include "literals.h"
#include "bmp_color.h"
// obsluga karty SD - biblioteki FATFS
#include "diskio.h"
#include "ff.h"

#define PGM_GETSTR(str, idx) (char *)pgm_read_word(&str[idx])

void touch_calibration(void);
void load_picture(const char *file_name, uint16_t x, uint16_t y);
void set_layout(void);
void SDTimerInit(void)
{
	OCR0 = F_CPU / 1024 / 100 - 1;
	TCCR0 = _BV(WGM01) | _BV(CS02) | _BV(CS00);
	TIMSK = _BV(OCIE0);
}
ISR(TIMER0_COMP_vect)
{
	disk_timerproc();
}

char driver_id_a[10];
uint16_t drawColor = GREEN;
//FATFS fatFs;							// obiekt do ktorego zaladuje sie plik

int main(void)
{
    ILI9341_init();							// inicjalizacja wyswietlacza
	XPT2046_init_io();						// inicjalizacja dotyku
	XPT2046_rd_ee_cal();					// martryca kalibracji
	SDTimerInit();							// Inicjacja timera do karty SD
	sei();									// uruchomienie przerwan

	PORTD |= _BV(PD3);						// To jest chyba od card detect, wiec mozna to spiac z 5V lub GND - musze wykminic
	ILI9341_set_rotation(LANDSCAPE);		// ustawienie obrazu w poziomie
	//touch_calibration();					// odpalane jednorazowo aby skalibrowac dotyk i obraz.
/*
//	Test kolorow tla.	
	ILI9341_cls(BLUE);
	_delay_ms(500);
	ILI9341_cls(RED);
	_delay_ms(500);
	ILI9341_cls(BRED);
	_delay_ms(500);
	ILI9341_cls(GRED);
	_delay_ms(500);
	ILI9341_cls(LGRAYBLUE);
	_delay_ms(500);
	ILI9341_cls(CYAN);
	_delay_ms(500);
	ILI9341_cls(GRAYBLUE);
	_delay_ms(500);
	ILI9341_cls(BLACK);
	_delay_ms(1000);
// Rozne czcionki					
	ILI9341_set_font((font_t) {font16x16, 16, 16, YELLOW, TRANSPARENT});	// ale tu pewnie cos trzeba zaimportowac jeszcze
	ILI9341_txt(1, 1, "Daniel");
	ILI9341_txt(1, 20, "Font 16x16");
	_delay_ms(1000);
	ILI9341_set_font((font_t) {font5x7, 5, 7, YELLOW, TRANSPARENT});
	ILI9341_txt(1, 40, "Font 5x7");
	_delay_ms(1000);
	ILI9341_set_font((font_t) {font8x12, 8, 12, CYAN, TRANSPARENT});
	ILI9341_txt(1, 50, "Font 8x12");
	_delay_ms(1000);
	ILI9341_set_font((font_t) {font8x8, 8, 8, MAGENTA, TRANSPARENT});
	ILI9341_txt(1, 70, "Font 8x8");
	_delay_ms(1000);
	ILI9341_cls(BLACK);
// Rozne linie	
	ILI9341_draw_line(10, 10, 200, 10, MAGENTA);
	ILI9341_draw_fast_line(50, 25, 100, RED, DN_SLOPE);
	for (uint8_t i = 0; i<100; i += 5)	// linia przerywana - co 5 pixeli
	{
		ILI9341_draw_pixel(i, 120, CYAN);
	}
	for (uint8_t i = 0; i<100; i += 3)	// linia przerywana - co 3 pixele
	{
		ILI9341_draw_pixel(i, 140, MAGENTA);
	}
	_delay_ms(1000);
	
	ILI9341_draw_circle(100, 100, 50, false, YELLOW);		// rysowanie okregu - false jest od wypelniania ksztaltu
	ILI9341_draw_circle(200, 100, 50, true, GREEN);			// rysowanie kola - true = wypelnienie
	_delay_ms(1000);
	
	uint16_t polygon[4][2] = 
	{
		{30, 40},
		{100, 70},
		{180, 140},
		{320, 240}
	};
	
	ILI9341_draw_polygon(*polygon, 3, RED, 1);
	_delay_ms(1000);
	
	ILI9341_draw_triangle(0, 0, 200, 220, 100, 200, MAGENTA);	//trojkat
	*/

	set_layout();
	
	// rysowanie grafiki
//	load_picture("GRAFIKA.BIN", 0, 0);
	//ILI9341_draw_color_bmp(120, 100, 32, 32, suzuki);
	
    while (1) 
    {
		XPT2046_rd_touch();					// odczyt wspolrzednych na digitizerze
		
		if (touch.ok)
		{
			ILI9341_draw_pixel(touch.x_cal, touch.y_cal, drawColor);
			ILI9341_draw_pixel(touch.x_cal - 1, touch.y_cal, drawColor);
			ILI9341_draw_pixel(touch.x_cal, touch.y_cal - 1, drawColor);
			ILI9341_draw_pixel(touch.x_cal, touch.y_cal + 1, drawColor);
			
			// instrukcja IF do zrobienia aktywnych pol pod guziki.
			if(touch.y_cal < 30)
			{
				if (touch.x_cal > 10 && touch.x_cal < 50)
					drawColor = RED;
				if (touch.x_cal > 60 && touch.x_cal < 100)
					drawColor = GREEN;
				if (touch.x_cal > 110 && touch.x_cal < 150)
					drawColor = BLUE;
				if (touch.x_cal > 160 && touch.x_cal < 200)
					drawColor = YELLOW;
				if (touch.x_cal > 210 && touch.x_cal < 250)
					drawColor = CYAN;
				if (touch.x_cal > 260 && touch.x_cal < 300)
					{
						ILI9341_cls(BLACK);
						//set_layout();
						load_picture("ZOLW.BIN", 0, 0);		
					}
			}
		}
    }
}

void set_layout(void)
{
	ILI9341_draw_rectangle(5, 30, 315, 235, YELLOW);			// prostokat
	ILI9341_draw_fast_rect(10, 5, 40, 20, true, RED);			// czerwony
	ILI9341_draw_fast_rect(60, 5, 40, 20, true, GREEN);		// zielony
	ILI9341_draw_fast_rect(110, 5, 40, 20, true, BLUE);		// niebieski
	ILI9341_draw_fast_rect(160, 5, 40, 20, true, YELLOW);		// zolty
	ILI9341_draw_fast_rect(210, 5, 40, 20, true, CYAN);		// cyan
	ILI9341_draw_fast_rect(260, 5, 40, 20, true, LGRAY);	// RESET
	ILI9341_draw_line(260, 5, 300, 25, RED);
	ILI9341_draw_line(300, 5, 260, 25, RED);				// iksik	
}

void touch_calibration(void)
{
	ILI9341_cls(BLACK);
	ILI9341_set_font((font_t) {font16x16, 16, 16, YELLOW, BLACK});
	int32_t x, y;
	point_t sample_points[3];
	
	for (uint8_t cal_step = 0; cal_step < 3; cal_step++)		// beda 3 punkty kalibracyjne
	{
		ILI9341_txt_P(0, 120, PGM_GETSTR(calibration_txt, cal_prompt_idx));
		eeprom_read_block(&x, &ee_cal_points[cal_step].x, sizeof(x));
		eeprom_read_block(&y, &ee_cal_points[cal_step].y, sizeof(y));
		ILI9341_draw_circle(x, y, 5, true, WHITE);			// rysuje kolko kalibracyjne w tych wspolrzednych
		
		do 
		{
			do 
			{
				XPT2046_rd_touch();
			} 
			while (!touch.ok);
			
			sample_points[cal_step].x = touch.x_raw;		// pobiera wspolrzedna X
			sample_points[cal_step].y = touch.y_raw;		// pobiera wspolrzedna Y
			
			if(labs(x - touch.x_raw) > MAX_CAL_ERROR || labs(y - touch.x_raw) > MAX_CAL_ERROR)
			{
				ILI9341_set_font((font_t) {font16x16, 16, 16, RED, BLUE});
				ILI9341_txt_P(0, 120, PGM_GETSTR(calibration_txt, cal_bad_idx));
			}
			else
			{
				ILI9341_set_font((font_t) {font16x16, 16, 16, GREEN, BLUE});
				ILI9341_txt_P(0, 120, PGM_GETSTR(calibration_txt, cal_ok_idx));
			}
		} 
		while (labs(x - touch.x_raw) > MAX_CAL_ERROR || labs(y - touch.y_raw) > MAX_CAL_ERROR);
		_delay_ms(500);
		ILI9341_draw_circle(x, y, 5, true, BLACK);		// usuwa stare kolko kalibracyjne: zamalowuje je na czarno
	}
	set_cal_matrix(sample_points);						// tworzy macierz kalibracyjna
	XPT2046_wr_ee_cal();
	ILI9341_set_font((font_t) {font16x16, 16, 16, YELLOW, BLUE});
	ILI9341_txt_P(0, 120, PGM_GETSTR(calibration_txt, cal_end_idx));
}

void load_picture( const char *file_name, uint16_t x, uint16_t y)																
{
	FATFS fatFs;
	ILI9341_set_font((font_t) {font16x16, 16, 16, YELLOW, TRANSPARENT});

	while(PIND & _BV(PIND3) );
	FRESULT fresult = f_mount(&fatFs, "", 1);
		
	if(fresult == FR_OK)
	{
		FIL fp;		// otwieramy plik do odczytu
		fresult = f_open(&fp, file_name, FA_READ);
		if (fresult == FR_OK)
		{
			char buffer[512];
			UINT bytes_read;
			uint8_t sector = 0;		// First sector										
			uint8_t start_pos;
			
			do
			{
				bytes_read = 0;											// Bytes read so far
				f_read(&fp, buffer, sizeof(buffer), &bytes_read);			// Read part of the file to buffer (sector 512b)

				if (!(sector))											// If first (zero) sector
				{
					ILI9341_set_window(x, y, buffer[0] + (buffer[1] << 8) - 1, buffer[2] + (buffer[3] << 8) - 1);	// Set picture size
					start_pos = 4;										// Data start at index 4
				}
				else start_pos = 0;										// Data start at index 0

				// Check if it is full sector or part of it
				for (uint16_t pixel = start_pos; pixel < (bytes_read % sizeof(buffer) ? bytes_read % sizeof(buffer) : sizeof(buffer)); pixel += 2)
				{
					ILI9341_push_color(buffer[pixel] + (buffer[pixel + 1] << 8));
				}
				sector++;
			}
			while (!(bytes_read % sizeof(buffer)));
			f_close(&fp);
		}
		else
		{
			ILI9341_txt(10, 5, "Blad pliku");
		}
	}
	else
	{
		ILI9341_txt(10, 5, "Blad karty");
	}
	// skasowana linia - oczekiwanie na wyjecie karty, a to zablokowaloby program.
}
	


