// Techatronic.com  
 #include <Wire.h>           
 #include <dht.h>
 #include <LiquidCrystal_I2C.h>    
 #define dht_apin A0
 #include <SoftwareSerial.h>
 SoftwareSerial mySerial(9, 10);//tx, rx


void SendMessage();

 LiquidCrystal_I2C lcd(0x27,20,3);   
  int val = 0 ;  
  int temp=0,i=0;  
  char str[15];  
  dht DHT;
  int smokeA0 = A1;
  int sensorThres = 230;
 void setup()  
 {  
    mySerial.begin(9600); // Setting the baud rate of GSM Module
    Serial.begin(9600);  // Setting the baud rate of Serial Monitor (Arduino)
    DHT.read11(dht_apin);
    pinMode(smokeA0, INPUT);
    lcd.init();      
    lcd.backlight();  
    pinMode(4,INPUT);  // Flame Sensor  
    pinMode(5,OUTPUT); // Led  
    pinMode(6,OUTPUT); // Buzzer  
    pinMode(8,OUTPUT); // Water
    lcd.setCursor(0,0);  
    lcd.print(" GSM Base Fire     ");  
    lcd.setCursor(0,1);  
    lcd.print("Security System ");  
    delay(2000);  
    lcd.clear();  
    Serial.println("AT+CNMI=2,2,0,0,0");  
    delay(500);  
    Serial.println("AT+CMGF=1");  
    delay(1000);  
 }  
 void loop()  
 {  
  int analogSensor = (analogRead(smokeA0));

  if(temp==1)  
  {    
   temp=0;  
   i=0;  
   delay(1000);  
  }  
   val = digitalRead(4); // pir sensor output pin connected  
   Serial.println(analogSensor);
   //Serial.println(val); // see the value in serial mpnitor in Arduino IDE  
   delay(100);  
  if(val == 0 || analogSensor > sensorThres )  
   {  
   Serial.print("\r");  
   delay(1000);           
   Serial.print("AT+CMGF=1\r");  
   lcd.clear(); 
   lcd.setCursor(0,0);  
   digitalWrite( 8,HIGH);
   lcd.print(" Fire Detected     ");  
   lcd.setCursor(0,1);  
   lcd.print("   Be Safe    ");
   SendMessage();
   for(int i=0;i<10;i++)
   {
    siren();
    noTone(6);
    }   
   /*Replace XXXXXXXXXX to 10 digit mobile number & ZZ to 2 digit country code*/  

 }  
 else  
  {  
     digitalWrite( 5,LOW); // led  
     digitalWrite( 6,LOW); // Buzzer 
     digitalWrite( 8,LOW); // Water
     DHT.read11(dht_apin); 
     lcd.setCursor(0,0); 
     lcd.print("      FIRE NOT    "); 
     lcd.setCursor(0,1); 
     lcd.print("      DETECTED   "); 
     lcd.setCursor(0,2);
     lcd.print(" Humidity : ");
     lcd.print(DHT.humidity);
     lcd.print(" %");
     lcd.setCursor(0,3);
     lcd.print(" Temp     : "); 
     lcd.print(DHT.temperature);
     lcd.print(" ");
     lcd.print(char(223));
     lcd.print("C"); 
  }  
  }  
 void siren()
 {
   for(i=700;i<800;i++){   // for police siren
  tone(6,i);
  delay(15);
  }
  for(i=800;i>700;i--){
  tone(6,i);
  delay(15);
  }
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
 
  
