#define F_CPU 1000000L
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
//--------------------------------------------
unsigned char R[4] = {'s','t','A','r'}; 
volatile short cnt=0;                   
unsigned char n_count=0;
uint8_t consider_Up = 0;   // Для подсчета звона кнопки
uint8_t consider_Down = 0; // Для подсчета звона кнопки
uint8_t prog_step = 2;     // Шаг подсчета  
const uint8_t bounce = 30; // Значение дребезга кнопки, можно править и подбирать !

// Учим зажигать цифры и буквы
void segchar(unsigned char ch)
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
		
		case '-':  PORTB =  0b11011111; break;
		default:   PORTB =  0b11111111; break; // Для остальных значений отключаем
	}
}

// Выключить все
void turnoffall(void)
{
	DDRC  &= ~0b00001111; /* Аноды цифр по умолчанию на вход. Отключаем ток */
	PORTC &= ~0b00001111; /* Значени по умолчанию. Убираем остатки тока через подтяжку, если она есть */
	
	PORTB = 0b11111111; /* Тушим свет для катодов */
	DDRB  = 0b11111111; /* Инициировать настройки портов. Стоит дешевле, чем приносит пользы */
}

// Включаем требуемый разряд
void turnon(unsigned char seg)
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

// Заводим таймер 
void timer_ini(void)
{
	TCCR1B |= (1<<WGM12); // Устанавливаем режим СТС (сброс по совпадению)
	TIMSK |= (1<<OCIE1A); // Устанавливаем бит разрешения прерывания 1ого счетчика по совпадению с OCR1A(H и L)
	OCR1AH = 0b00000000;  // Записываем в регистр число для сравнения
	OCR1AL = 0b01100100;
	TCCR1B |= (1<<CS10);  // Установим делитель.
}

// Действия при прерывание по таймеру
ISR (TIMER1_COMPA_vect)
{
	static unsigned char new_state=0; // Переменные энкодера
	static unsigned char old_state=0; // Переменные энкодера

	new_state = (PINC & 0b00110000) >> 4;
	
	// Если вдруг обнаружим низкое состояние (кнопка нажата), то обнулим счётчик
	if(!(PIND & 0b00010000)) cnt = 0; 
	
	switch(old_state | new_state)
	{
		case 0x01: case 0x0e:
		cnt++;
		break;
		case 0x04: case 0x0b: 
		cnt--;
		break;
	}
	old_state = new_state << 2;
	
	// Выводим цифру на индикатор 
	turnoffall();        // Отключаем все
	segchar(R[n_count]);
	turnon(n_count);     // Включаем ток 
	if (++n_count >= sizeof(R) / sizeof(R[0])) n_count = 0;
	
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
	//--------------------------------------------
}
// Меняем точку зрения (меняем очередность сегментов)
uint8_t P (uint8_t x)
{
	return ((sizeof(R) / sizeof(R[0])) - 1) - x;
}

// Присваиваем каждому сигменту своё число
void ledprint(unsigned int number, unsigned char Rl[4])
{
	for (uint8_t i = 0; i < sizeof(R) / sizeof(R[0]); i++)
	{
		// Rl[P(i)] возвращяем удобное для нас значения 
		Rl[P(i)] = '0' + number % 10;  
		if(i && !number) Rl[P(i)] = ' ';
		number /= 10;
	}
}

// Начинаем начинать 
int main(void)
{
	unsigned char Rx[4];    // Обьявим локальный массив для разрядов 
	short cnt_local = 0;    // Для сохранения подсчета внутри критической секции
	bool update;            // Флаг перерисовки дисплея
	bool klik_Down = false; // Состаяние кнопки Down
	bool klik_Up = false;   // Состаяние кнопки Up
	DDRC  &= ~0b00110000;   // Вход энкодера и кнопки
	PORTC |=  0b00110000;   // Подтяжка
	DDRD  &= ~0b00010000;   // Кнопка сброса
	PORTD |=  0b00010000;   // Кнопка сброса
	DDRD  &= ~0b00001000;   // Кнопка down
	PORTD |=  0b00001000;   // Кнопка down
	DDRD  &= ~0b00000100;   // Кнопка up
	PORTD |=  0b00000100;   // Кнопка up
	//--------------------------------------------
	timer_ini();
	turnoffall();
	//--------------------------------------------
	sei();                // Разрешаем прерывания. Это важно!
	_delay_ms(10);        // Задержка на вывод заставки
	update = true;        // Принудительно обновим дисплей в первый раз
	//--------------------------------------------	
	while(1)
	{
		cli();                // Открываем критическую секцию 
		if (cnt_local != cnt) // Если счетчик изменился?
		{
			// Обновляем Локальную копию счетчика + шаг программы 
			if (prog_step > 2) cnt_local = cnt / (prog_step / 2); 
			else cnt_local = cnt;
			update = true;    // Устанавливаем флаг перерисовки дисплея
		}
		sei();                // Закрываем критическую секцию
		
		//--------------------------------------------
		cli();                                          // Открываем критическую секцию
		if (consider_Down == bounce) klik_Down = true;  // Будем считать что кнопка нажата 
		else 
		{
		if (consider_Down == 0) klik_Down = false;      // Кнопка не тронута 	
		}
		sei();                                          // Закрываем критическую секцию
		//--------------------------------------------
		cli();                                          // Открываем критическую секцию
		if (consider_Up == bounce) klik_Up = true;      // Будем считать что кнопка нажата
		else
		{
			if (consider_Up == 0) klik_Up = false;      // Кнопка не тронута
		}
		sei();                                          // Закрываем критическую секцию
		//--------------------------------------------
		if (update)           // Флаг установлен?
		{
			update = false;   // Сбрасывем флаг
			
			/*
			Пересчитывам буфер.
			Вот здесь из отрицательного числа делаем положительное 
			что бы не зависть от направления вращения энкодера после включения
			*/
			ledprint(cnt_local >= 0 ? cnt_local : -cnt_local, Rx); // Шедевр простоты !!!
			
			//--------------------------------------------
			cli();            // Открываем критическую секцию 
			for (int w = 0; w < sizeof(R) / sizeof(R[0]);  w++)
			{
				R[w] = Rx[w]; // Обновляем Локальную копию массива регистров 
			}
			sei();            // Закрываем критическую секцию
		}
		
		//--------------------------------------------
		// Добавил вывод на экран программы и систему ее переключения
		// Получилось очень растянуто, уверен что можно перепесать более грамотно 
		
		while ((klik_Up || klik_Down) && (prog_step == 2 || prog_step == 4 || prog_step == 8))
		{
			if(klik_Up && prog_step < 8)
			{
				klik_Up = false; // Сбрасываем флаг, иначи не прекрашает переключать
				prog_step *= 2;
				R[0] = 'P';
				R[1] = 'r';
				R[2] = 'g';
				R[3] = '0' + prog_step;
				_delay_ms(10);
				update = true; // Принудительно обновляем экран
			}
			if(klik_Down && prog_step > 2)
			{
				klik_Down = false;
				prog_step /= 2;
				R[0] = 'P';
				R[1] = 'r';
				R[2] = 'g';
				R[3] = '0' + prog_step;
				_delay_ms(10);
				update = true;
			}
			
			
		}
		
// 		if(klik_Up && prog_step == 2)
// 		{
// 			klik_Up = false; // Сбрасываем флаг, иначи не прекрашает переключать 
// 			prog_step = 4;
// 			R[0] = 'P';
// 			R[1] = 'r';
// 			R[2] = 'g';
// 			R[3] = '0' + prog_step;
// 			_delay_ms(10);
// 			update = true; // Принудительно обновляем экран
// 		}
// 		if(klik_Up && prog_step == 4)
// 		{
// 			klik_Up = false;
// 			prog_step = 8;
// 			R[0] = 'P';
// 			R[1] = 'r';
// 			R[2] = 'g';
// 			R[3] = '0' + prog_step;
// 			_delay_ms(10);
// 			update = true;
// 		}
// 		if(klik_Down && prog_step == 8)
// 		{
// 			klik_Down = false;
// 			prog_step = 4;
// 			R[0] = 'P';
// 			R[1] = 'r';
// 			R[2] = 'g';
// 			R[3] = '0' + prog_step;
// 			_delay_ms(10);
// 			update = true;
// 		}
// 		if(klik_Down && prog_step == 4)
// 		{
// 			klik_Down = false;
// 			prog_step = 2;
// 			R[0] = 'P';
// 			R[1] = 'r';
// 			R[2] = 'g';
// 			R[3] = '0' + prog_step;
// 			_delay_ms(10);
// 			update = true;
// 		}
		//--------------------------------------------
		//_delay_ms(1); // Не стоит обновлять дисплей слишком часто.
	}
}