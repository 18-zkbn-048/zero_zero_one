/*
 * disp.c
 *
 * Created: 30.05.2021 18:53:09
 *  Author: Белоусов Д.В.
 */ 
#define F_CPU 1000000L
#include "disp.h"
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

static volatile short consider_Up = 0;   // Для подсчета звона кнопки
static volatile short consider_Down = 0; // Для подсчета звона кнопки
static volatile short bounce = 10;       // Значение дребезга кнопки, можно править и подбирать !
bool retention = true; // Защитимся от удержании кнопки
bool klik_Down = false; // Состаяние кнопки Down
bool klik_Up = false;   // Состаяние кнопки Up


static unsigned char n_count=0;
unsigned char R[4]; // Массив под индикатор
static unsigned char ticker[] = {' ',' ',' ','s','t','A','r','t',' ','s','C','0','r','E',' ','v','1','0','0'}; 
bool update;            // Флаг перерисовки дисплея

void disp_init(void)
{
	DDRC  &= ~0b00001111; /* Аноды цифр по умолчанию на вход. Отключаем ток */
	PORTC &= ~0b00001111; /* Значени по умолчанию. Убираем остатки тока через подтяжку, если она есть */
	DDRB  = 0b11111111; /* Инициировать настройки портов. Стоит дешевле, чем приносит пользы */
	PORTB = 0b11111111; /* Тушим свет для катодов */
	
	TCCR0 = 0b00000001; // установили пред-делитель
	TIMSK |= (1<<0);
	TCNT0 = 0; // 255
}

// Учим зажигать цифры и буквы
static void segchar(unsigned char ch)
{
	switch (ch)
	{
		case ' ':  PORTB = 0b11111111; break;
		case '0':  PORTB = 0b00101000; break;
		case '1':  PORTB = 0b10101111; break;
		case '2':  PORTB = 0b10011000; break;
		case '3':  PORTB = 0b10001010; break;
		case '4':  PORTB = 0b00001111; break;
		case '5':  PORTB = 0b01001010; break;
		case '6':  PORTB = 0b01001000; break;
		case '7':  PORTB = 0b10101110; break;
		case '8':  PORTB = 0b00001000; break;
		case '9':  PORTB = 0b00001010; break;
		
		case 'A':  PORTB = 	0b00001100; break;
		case 'b':  PORTB = 	0b01001001; break;
		case 'C':  PORTB = 	0b01111000; break;
		case 'd':  PORTB = 	0b10001001; break;
		case 'E':  PORTB = 	0b01011000; break;
		case 'F':  PORTB = 	0b01011100; break;
		case 'G':  PORTB = 	0b01101000; break;
		case 'H':  PORTB = 	0b00001101; break;
		case 'i':  PORTB = 	0b11111000; break;
		case 'j':  PORTB = 	0b11101010; break;
		case 'k':  PORTB = 	0b00011101; break;
		case 'L':  PORTB = 	0b01111001; break;
		case 'm':  PORTB = 	0b11001100; break;
		case 'n':  PORTB = 	0b11001101; break;
		case 'o':  PORTB = 	0b11001000; break;
		case 'P':  PORTB = 	0b00011100; break;
		case 'g':  PORTB = 	0b00001110; break;
		case 'r':  PORTB = 	0b11011101; break;
		case 's':  PORTB = 	0b01001010; break;
		case 't':  PORTB = 	0b01011001; break;
		case 'U':  PORTB = 	0b00101001; break;
		case 'v':  PORTB = 	0b11101000; break;
		case 'w':  PORTB = 	0b00011011; break;
		case 'x':  PORTB = 	0b11101101; break;
		case 'y':  PORTB = 	0b00001011; break;
		case 'z':  PORTB = 	0b11011010; break;
		case '.':  PORTB =  0b11110111; break;
		case '-':  PORTB =  0b11011111; break;
		default:   PORTB =  0b11111111; break; // Для остальных значений отключаем
	}
}

// Выключить все
static void turnoffall(void)
{
	PORTC &= ~0b00001111; /* Значени по умолчанию. Убираем остатки тока через подтяжку, если она есть */
	PORTB = 0b11111111; /* Тушим свет для катодов */
}

// Включаем требуемый разряд
static void turnon(unsigned char seg)
{
	switch (seg)
	{
		case 0:
		PORTC |= (1 << PORTC1); // Уровень 1
		DDRC  |= (1 << PORTC1); // Выход
		break;
		
		case 1:
		PORTC |= (1 << PORTC2); // Уровень 1
		DDRC  |= (1 << PORTC2); // Выход
		break;
		
		case 2:
		PORTC |= (1 << PORTC3); // Уровень 1
		DDRC  |= (1 << PORTC3); // Выход
		break;
		
		case 3:
		PORTC |= (1 << PORTC0); // Уровень 1
		DDRC  |= (1 << PORTC0); // Выход
		break;
	}
}
//
ISR (TIMER0_OVF_vect) // Вывод по прерыванию 
{
	turnoffall();        // Отключаем все
	segchar(R[n_count]);
	turnon(n_count);     // Включаем ток
	if (++n_count >= sizeof(R) / sizeof(R[0])) n_count = 0;
	
	//--------------------------------------------
	if ((PIND & 0b00001000) && (PIND & 0b00000100)) retention = true; // Кнопки отжаты, можно сбросить состояние удержания

	//--------------------------------------------
	if(!(PIND & 0b00001000) && consider_Down < bounce) consider_Down++; // Будем инкрементировать нажатое состояние кнопки
	else
	{
		if (consider_Down > 0) consider_Down--;                         // Будем декрементировать если кнопка отпущена
	}
	//--------------------------------------------
	if(!(PIND & 0b00000100) && consider_Up < bounce) consider_Up++;     // Будем инкрементировать нажатое состояние кнопки
	else
	{
		if (consider_Up > 0) consider_Up--;                             // Будем декрементировать если кнопка отпущена
	}
}

// Меняем точку зрения (меняем очередность сегментов)
static uint8_t P (uint8_t x)
{
	return ((sizeof(R) / sizeof(R[0])) - 1) - x;
}

// Присваиваем каждому сигменту своё число
void disp_out(unsigned int number)
{
	static unsigned char Rl[4];
	for (uint8_t i = 0; i < sizeof(R) / sizeof(R[0]); i++)
	{
		//[P(i)] возвращяем удобное для нас значения
		Rl[P(i)] = '0' + number % 10;
		if(i && !number) Rl[P(i)] = ' ';
		number /= 10;
	}
	
	cli();            // Открываем критическую секцию
	for (int w = 0; w < sizeof(R) / sizeof(R[0]);  w++)
	{
		R[w] = Rl[w]; // Обновляем Локальную копию массива регистров
	}
	sei();            // Закрываем критическую секцию
}

void ledprintRun(void)
{
	static unsigned char Rj[4];    // Обьявим локальный массив для разрядов вывода программы
	
	for (uint8_t n = 0; n < sizeof(ticker) / sizeof(ticker[0]); n++)
	{
		Rj[0] = ticker[n];
		Rj[1] = ticker[n + 1 > (sizeof(ticker) / sizeof(ticker[0]) -1) ? 0 : n + 1];
		Rj[2] = ticker[n + 2 > (sizeof(ticker) / sizeof(ticker[0]) -1) ? 0 : n + 2];
		Rj[3] = ticker[n + 3 > (sizeof(ticker) / sizeof(ticker[0]) -1) ? 0 : n + 3];
		
		cli();            // Открываем критическую секцию
		for (int x = 0; x < sizeof(R) / sizeof(R[0]);  x++)
		{
			R[x] = Rj[x]; // Обновляем Локальную копию массива регистров
		}
		sei();            // Закрываем критическую секцию
		
		_delay_ms(5);
	}
	update = true;
}

void buttons_ringing (void) // Обрабатываем звон контакта кнопки
{
	if ((consider_Down == bounce) && retention) klik_Down = true;  // Будем считать что кнопка нажата
	//--------------------------------------------
	if ((consider_Up == bounce) && retention) klik_Up = true;      // Будем считать что кнопка нажата
}
