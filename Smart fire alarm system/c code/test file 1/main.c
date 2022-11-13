#include <avr/io.h>
#include <string.h>

#define E   PA7
#define RS   PC0

void display(char string[16], char LineNo);
void displaybyte(char D);
void dispinit(void);
void epulse(void);
void delay_ms(unsigned int de);


char mystr[6];
int Temperature,setpoint,Smoke,SmokeSet;
unsigned char u8_data;
//=================================================================
//        Main Function
//=================================================================
int main(void)
{
	setpoint=60;  //Temprature Limit For detection of Fire
	SmokeSet=100; //Smoke Setpoint
	char Flag;

	DDRC = 0xF1;  //Set LCD Port Direction
	DDRA = 0xE0;
	PORTC = 0x06; //Pull up for switches

	delay_ms(600);  //Initiaize LCD
	dispinit();
	delay_ms(200);

	display("Temperature:32 C",1);
	display("Smoke:55 PPM    ",2);
	while(1)
	{
		//Measure Temprature and Display
		ADMUX=0xE5;
		ADCSRA=0xC7;  //Internal Referance 2.56V
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
		ADCSRA=0xC7;  //Internal Referance 2.56V
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
			//Over Temprature SMS
			if(Flag==0)
			{
				Flag=1;
				PORTC &=~(1<<PC5);  //Turn of  Electrical Supply
				PORTC |=(1<<PC4);  //Turn on buzzer
			}
		}
			else
			{
				Flag=0;
				PORTC |=(1<<PC5);   //Keep on Electrical Supply
				PORTC &=~(1<<PC4);   //Turn off buzzer
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
	
	PORTC &= ~(1<<RS);           // RS=0
	for (count = 0; count <= 9; count++)
	{
		displaybyte(init[count]);
	}
	PORTC |= 1<<RS;    //RS=1
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
	//D4=PA6
	//D5=PA5
	//D6=PC7
	//D7=PC6
	//data is in Temp Register
	char K1;
	K1=D;
	K1=K1 & 0xF0;
	K1=K1 >> 4;  //Send MSB
	
	PORTD &= 0x9F;  //Clear data pins
	PORTC &= 0x3F;
	
	if((K1 & 0x01)==0x01){PORTD |= (1<<PA6);}
	if((K1 & 0x02)==0x02){PORTD |= (1<<PA5);}
	if((K1 & 0x04)==0x04){PORTC |= (1<<PC7);}
	if((K1 & 0x08)==0x08){PORTC |= (1<<PC6);}

	epulse();

	K1=D;
	K1=K1 & 0x0F;  //Send LSB
	PORTA &= 0x9F;  //Clear data pins
	PORTC &= 0x3F;

	if((K1 & 0x01)==0x01){PORTA |= (1<<PA6);}
	if((K1 & 0x02)==0x02){PORTA |= (1<<PA5);}
	if((K1 & 0x04)==0x04){PORTC |= (1<<PC7);}
	if((K1 & 0x08)==0x08){PORTC |= (1<<PC6);}
	epulse();
}

//=================================================================
//        Display Line on LCD at desired location Function
//=================================================================
void display(char string[16], char LineNo)
{
	int len,count;

	PORTC &= ~(1<<RS);           // RS=0 Command Mode

	if(LineNo==1)
	{
		displaybyte(0x80);  //Move Coursor to Line 1
	}
	else
	{
		displaybyte(0xC0);  //Move Coursor to Line 2
	}
	PORTC |= (1<<RS);           // RS=1 Data Mode



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

