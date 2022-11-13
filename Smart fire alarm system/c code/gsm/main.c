
#include <SoftwareSerial.h>
SoftwareSerial mySerial(9, 10);//tx, rx


void SendMessage();


void setup()
{
	mySerial.begin(9600); // Setting the baud rate of GSM Module
	Serial.begin(9600); // Setting the baud rate of Serial Monitor (Arduino)


	Serial.println();
	delay(100);
}
void loop()
{

	SendMessage();


}

void SendMessage()
{
	Serial.println("Sending Message Detected");
	mySerial.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
	delay(1000); // Delay of 1000 milli seconds or 1 second
	mySerial.println("AT+CMGS=\"+94776203223\"\r"); // Replace x with mobile number +94
	delay(1000);
	mySerial.println("Fire detected");// The SMS text you want to send
	delay(100);
	mySerial.println((char)26);// ASCII code of CTRL+Z
	delay(1000);
}

