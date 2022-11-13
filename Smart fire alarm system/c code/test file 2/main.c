
#include <avr/io.h>
#include <string.h>
#include <stdio.h>

#define E   PD7
#define RS   PB0

void display(char string[16], char LineNo);
void displaybyte(char D);
void dispinit(void);
void epulse(void);
void delay_ms(unsigned int de);

void USART_Transmit(char data );
void senddata(char string[16]);
void USART_Init();
void USART_Receive();
void sendSMS();

char mystr[6];
int Temperature,setpoint,Smoke,SmokeSet;
unsigned char u8_data;
//=================================================================
//        Main Function
//=================================================================
int main(void)
{
	setpoint=60;  //Temperature Limit For detection of Fire
	SmokeSet=100; //Smoke Set point
	char Flag;

	DDRB = 0xF1;  //Set LCD Port Direction
	DDRD = 0xE0;
	PORTB = 0x06; //Pull up for switches

	delay_ms(500);  //Initialize LCD
	dispinit();
	delay_ms(200);

	USART_Init();  //9600 Baud rate at internal oscillator Clock 1MHz

	display("Temperature:32 C",1);
	display("Smoke:55 PPM    ",2);
	while(1)
	{
		//Measure Temperature and Display
		ADMUX=0xE5;
		ADCSRA=0xC7;  //Internal Reference 2.56V
		while (!(ADCSRA & (1<<ADIF)));
		Temperature=ADCH;
		ADCSRA |= 1<<4;
		sprintf(mystr, "%03d", Temperature);
		display("Temperature:",1);
		displaybyte(mystr[1]);
		displaybyte(mystr[2]);
		displaybyte(0xDF);
		displaybyte('C');
		displaybyte(0x20);

		ADMUX=0xE4;   //Smoke Sensor
		ADCSRA=0xC7;  //Internal Reference 2.56V
		while (!(ADCSRA & (1<<ADIF)));
		Smoke=ADCH;   //Do some math here for calibration
		ADCSRA |= 1<<4;
		sprintf(mystr, "%03d", Smoke);
		display("Smoke:",2);
		displaybyte(mystr[0]);
		displaybyte(mystr[1]);
		displaybyte(mystr[2]);
		displaybyte('P');
		displaybyte('P');
		displaybyte('M');
		displaybyte(0x20);

		//Compare with Set Points and Send SMS
		if(Temperature>setpoint || Smoke>SmokeSet)
		{
			//Over Temperature SMS
			if(Flag==0)
			{
				sendSMS();
				Flag=1;
				PORTB &=~(1<<PORTB5);  //Turn of  Electrical Supply
				PORTB |=(1<<PORTB4);  //Turn on buzzer
			}
		}
		else
		{
			Flag=0;
			PORTB |=(1<<PORTB5);   //Keep on Electrical Supply
			PORTB &=~(1<<PORTB4);   //Turn off buzzer
		}
	}
}

//=================================================================
//        LCD Display Initialization Function
//=================================================================
void dispinit(void)
{
	int count;
	char init[]={0x43,0x03,0x03,0x02,0x28,0x01,0x0C,0x06,0x02,0x02};
	
	PORTB &= ~(1<<RS);           // RS=0
	for (count = 0; count <= 9; count++)
	{
		displaybyte(init[count]);
	}
	PORTB |= 1<<RS;    //RS=1
}


//=================================================================
//        Enable Pulse Function
//=================================================================
void epulse(void)
{
	PORTD |= 1<<E;
	delay_ms(1); //Adjust delay if required
	PORTD &= ~(1<<E);
	delay_ms(1); //Adjust delay if required
}


//=================================================================
//        Send Single Byte to LCD Display Function
//=================================================================
void displaybyte(char D)
{
	//D4=PD6
	//D5=PD5
	//D6=PB7
	//D7=PB6
	//data is in Temp Register
	char K1;
	K1=D;
	K1=K1 & 0xF0;
	K1=K1 >> 4;  //Send MSB
	
	PORTD &= 0x9F;  //Clear data pins
	PORTB &= 0x3F;
	
	if((K1 & 0x01)==0x01){PORTD |= (1<<PORTD6);}
	if((K1 & 0x02)==0x02){PORTD |= (1<<PORTD5);}
	if((K1 & 0x04)==0x04){PORTB |= (1<<PORTB7);}
	if((K1 & 0x08)==0x08){PORTB |= (1<<PORTB6);}

	epulse();

	K1=D;
	K1=K1 & 0x0F;  //Send LSB
	PORTD &= 0x9F;  //Clear data pins
	PORTB &= 0x3F;

	if((K1 & 0x01)==0x01){PORTD |= (1<<PORTD6);}
	if((K1 & 0x02)==0x02){PORTD |= (1<<PORTD5);}
	if((K1 & 0x04)==0x04){PORTB |= (1<<PORTB7);}
	if((K1 & 0x08)==0x08){PORTB |= (1<<PORTB6);}
	epulse();
}

//=================================================================
//        Display Line on LCD at desired location Function
//=================================================================
void display(char string[16], char LineNo)
{
	int len,count;

	PORTB &= ~(1<<RS);           // RS=0 Command Mode

	if(LineNo==1)
	{
		displaybyte(0x80);  //Move Cursor to Line 1
	}
	else
	{
		displaybyte(0xC0);  //Move Cursor to Line 2
	}
	PORTB |= (1<<RS);           // RS=1 Data Mode



	len = strlen(string);

	for (count=0;count<len;count++)
	{
		displaybyte(string[count]);
	}
}


//=================================================================
//        Delay Function
//=================================================================
void delay_ms(unsigned int de)
{
	unsigned int rr,rr1;
	for (rr=0;rr<de;rr++)
	{
		
		for(rr1=0;rr1<30;rr1++)   //395
		{
			asm("nop");
		}
		
	}
}


void USART_Transmit(char data )
{
	UDR0 = data;
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) )
	;
	/* Put data into buffer, sends the data */
	
}


void senddata(char string[16])
{
	int len,count;
	len = strlen(string);

	for (count=0;count<len;count++)
	{
		USART_Transmit(string[count]);
	}
}

void USART_Init()
{
	/* Set baud rate */
	UBRR0H = 0x00;  //12, 9600 Baud At 1MHz
	UBRR0L =12;
	//Set double speed enabled
	UCSR0A |= (1<<U2X0);
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1<<UMSEL01)|(1<<USBS0)|(3<<UCSZ00);
	//Set interrupt on RX
	//  UCSRB |= (1<<RXCIE);
}

void USART_Receive()
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXCIE0)) )
	;
	/* Get and return received data from buffer */
	u8_data=UDR0;
}

void sendSMS()
{
	senddata("AT+CMGD=1");
	USART_Transmit(13);
	USART_Transmit(10);
	delay_ms(1000);
	
	senddata("AT+CMGF=1");
	USART_Transmit(13);
	USART_Transmit(10);
	delay_ms(1000);
	
	senddata("AT+CMGW=");
	USART_Transmit(34);
	senddata("+94774740855");  //Enter Your Mobile number
	USART_Transmit(34);
	USART_Transmit(13);
	USART_Transmit(10);
	delay_ms(1000);
	
	senddata("Alert: Fire Detected");
	USART_Transmit(13);
	USART_Transmit(10);
	delay_ms(1000);
	senddata("Temperature:");
	
	ADMUX=0xE5;
	ADCSRA=0xC7;  //Internal Reference 2.56V
	while (!(ADCSRA & (1<<ADIF)));
	Temperature=ADCH;
	ADCSRA |= 1<<4;

	sprintf(mystr, "%03d", Temperature);
	USART_Transmit(mystr[1]);
	USART_Transmit(mystr[2]);
	USART_Transmit('C');
	USART_Transmit(13);
	USART_Transmit(10);

	senddata("Smoke Level:");
	ADMUX=0xE4;   //Smoke Sensor
	ADCSRA=0xC7;  //Internal Reference 2.56V
	while (!(ADCSRA & (1<<ADIF)));
	Smoke=ADCH;   //Do some math here for calibration
	ADCSRA |= 1<<4;
	sprintf(mystr, "%03d", Smoke);
	USART_Transmit(mystr[0]);
	USART_Transmit(mystr[1]);
	USART_Transmit(mystr[2]);
	USART_Transmit('P');
	USART_Transmit('P');
	USART_Transmit('M');
	USART_Transmit(13);
	USART_Transmit(10);


	delay_ms(1000);
	USART_Transmit(26); //Ctrl+Z
	delay_ms(1000);
	delay_ms(1000);

	senddata("AT+CMSS=1");
	USART_Transmit(13);
	USART_Transmit(10);
	delay_ms(1000);
}
