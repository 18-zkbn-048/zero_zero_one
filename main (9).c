#define F_CPU 1000000L
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <cmath>
//--------------------------------------------
unsigned char R[4] = {'s','t','A','r'}; 
volatile short cnt=0;                   
unsigned char n_count=0;

// ���� �������� ����� � �����
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
		default:   PORTB =  0b11111111; break; // ��� ��������� �������� ���������
	}
}

// ��������� ���
void turnoffall(void)
{
	DDRC  &= ~0b00001111; /* ����� ���� �� ��������� �� ����. ��������� ��� */
	PORTC &= ~0b00001111; /* ������� �� ���������. ������� ������� ���� ����� ��������, ���� ��� ���� */
	
	PORTB = 0b11111111; /* ����� ���� ��� ������� */
	DDRB  = 0b11111111; /* ������������ ��������� ������. ����� �������, ��� �������� ������ */
}

// �������� ��������� ������
void turnon(unsigned char seg)
{
	switch (seg)
	{
		case 0:
		PORTC |= (1 << PORTC1); // ������� 1
		DDRC  |= (1 << PORTC1); // �����
		break;
		
		case 1:
		PORTC |= (1 << PORTC2); // ������� 1
		DDRC  |= (1 << PORTC2); // �����
		break;
		
		case 2:
		PORTC |= (1 << PORTC3); // ������� 1
		DDRC  |= (1 << PORTC3); // �����
		break;
		
		case 3:
		PORTC |= (1 << PORTC0); // ������� 1
		DDRC  |= (1 << PORTC0); // �����
		break;
	}
}

// ������� ������ 
void timer_ini(void)
{
	TCCR1B |= (1<<WGM12); // ������������� ����� ��� (����� �� ����������)
	TIMSK |= (1<<OCIE1A); // ������������� ��� ���������� ���������� 1��� �������� �� ���������� � OCR1A(H � L)
	OCR1AH = 0b00000000;  // ���������� � ������� ����� ��� ���������
	OCR1AL = 0b01100100;
	TCCR1B |= (1<<CS10);  // ��������� ��������.
}

//������ ����� ������ (������ ����������� ���������)
uint8_t P (uint8_t x)
{
	/*
	if (x == 3) return 0;
	if (x == 2) return 1;
	if (x == 1) return 2;
	if (x == 0) return 3;
	*/
	/*
	uint8_t Q = 3;
	for (uint8_t i = 0; i < sizeof(R) / sizeof(R[0]); i++)
	{
		if (x == i) return --Q;
	}
	*/
	return ((sizeof(R) / sizeof(R[0]) - 1) - x;
}

// �������� ��� ���������� �� �������
ISR (TIMER1_COMPA_vect)
{
	static unsigned char new_state=0;
	static unsigned char old_state=0; 

	new_state = (PINC & 0b00110000) >> 4;
	if(!(PIND & 0b00010000)) cnt = 0; //���� ����� ��������� ������ ��������� ����� ������ (������ ������), �� ������� �������
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
	
	//--------------------------------------------
	turnoffall(); /* ��������� ��� */
	segchar(R[n_count]);
	turnon(n_count); /* �������� ��� */
	if (++n_count >= sizeof(R) / sizeof(R[0])) n_count = 0;
}

//����������� ������� �������� ��� �����
void ledprint(unsigned int number, unsigned char Rl[4])
{
	for (uint8_t i = 0; i < sizeof(R) / sizeof(R[0]); i++)
	{
		Rl[P(i)] = '0' + number % 10; // Rl[P(i)] ���������� ������� ��� ��� �������� 
		if(i && !number) Rl[P(i)] = ' ';
		number /= 10;
	}
}

// �������� �������� 
int main(void)
{
	unsigned char Rx[4];  // ������� ��������� ������ ��� �������� 
	short cnt_local = 0;  // ��� ���������� �������� ������ ����������� ������
	bool update;          // ���� ����������� �������
	DDRC  &= ~0b00110000; // ���� �������� � ������
	PORTC |=  0b00110000; // ���������
	DDRD  &= ~0b00010000; // ������� ��� ������
	PORTD |=  0b00010000; // ������� ��� ������
	//--------------------------------------------
	timer_ini();
	turnoffall();
	//--------------------------------------------
	sei(); // ��������� ����������. ��� �����!
	//--------------------------------------------
	_delay_ms(10); // �������� �� ����� ��������
	//--------------------------------------------
	update = true; // ������������� ������� ������� � ������ ���
	//--------------------------------------------	
	while(1)
	{
		cli(); // ��������� ����������� ������ 
		if (cnt_local != cnt) // ���� ������� ���������?
		{
			cnt_local = cnt;  // ��������� ��������� ����� ��������
			update = true;    // ������������� ���� ����������� �������
		}
		sei(); // ��������� ����������� ������
		
		//--------------------------------------------
		if (update) // ���� ����������?
		{
			update = false; // ��������� ����
			
			//--------------------------------------------
			/*
			������������ �����.
			��� ����� �� �������������� ����� ������ ������������� 
			��� �� �� ������� �� ����������� �������� �������� ����� ���������
			*/
			ledprint(cnt_local >= 0 ? cnt_local : -cnt_local, Rx);
			
			//--------------------------------------------
			cli(); // ��������� ����������� ������ 
			for (int w = 0; w < sizeof(R) / sizeof(R[0]);  w++)
			{
				R[w] = Rx[w]; // ��������� ��������� ����� ������� ��������� 
			}
			sei(); // ��������� ����������� ������
		}
		// �������� �� ����� �������� ������� ������� 
		//_delay_ms(1); // �� ����� ��������� ������� ������� �����.
	}
}