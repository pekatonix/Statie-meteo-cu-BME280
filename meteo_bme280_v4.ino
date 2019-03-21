/*!
 * Creat de Peter Gyorfi
 * Statie meteo cu senzor BME280 si DS18B20  pentru temperatura exterioara
 * Varianta 3
 * Targu Mures, 3/13/2019
 * @copyright GNU General Public License
 */

#include <DFRobot_BME280.h>
#include <LiquidCrystal.h>
#include <DallasTemperature.h>
#include <OneWire.h>         

#define SEA_LEVEL_PRESSURE  1013.25f //Periunea atmosferica la nivelul marii
#define BME_CS 10
#define ONE_WIRE_BUS 10 //pin DS18B20
#define TEMPERATURE_PRECISION1 11
OneWire oneWire(ONE_WIRE_BUS);      //initializam DS18B20 oneWire
DallasTemperature senzor(&oneWire); //initializam DallasTemperature(calcule pt DS18B20)

DFRobot_BME280 bme; //I2C

float pa, hum, alt;
float tempint = 0; //temperatura interioara
float tempext = 0; //temperatura exterioara DS18B20

LiquidCrystal lcd(8, 9, 4, 5, 6, 7); //RS,EN,D4,D5,D6,D7

byte grad[8] = {  //Simbol grad
	B01100,
	B10010,
	B10010,
	B01100,
	B00000,
	B00000,
	B00000,
	B00000,
};
/*
byte arrow_d[8] = {  //Simbol sageata jos
	B00100, 
	B00100, 
	B00100, 
	B00100, 
	B00100, 
	B11111, 
	B01110, 
	B00100 
}; 
*/
byte pic_sus[8] = {   //Simbol picatura sus
	B00000, 
	B11111, 
	B01110, 
	B00100, 
	B00000, 
	B00000, 
	B00000, 
	B00000 
}; 

byte pic_jos[8] = {   //picatura jos
	B00000,
	B00000,
	B00000,
	B00000,
	B11111,
	B01110,
	B00100,
	B00000
}; 

byte soare_st[8] = {   //Simbol soare stanga
	B00010,
	B01001,
	B00011,
	B00111,
	B10111,
	B00011,
	B01001,
	B00010
}; 

byte soare_dr[8] = {   //Simbol soare dreapta
	B01000,
	B10010,
	B11000,
	B11101,
	B11100,
	B11000,
	B10010,
	B01000
}; 

byte nor_st[8] = {   //Simbol nor stanga
  B00000,
  B00000,
  B00111,
  B01111,
  B11111,
  B01111,
  B00001,
  B00000
};

byte nor_dr[8] = {   //Simbol nor dreapta
  B00000,
  B00000,
  B11100,
  B11110,
  B11111,
  B11110,
  B10000,
  B00000
};

void setup() {
    Serial.begin(9600);
	senzor.begin();
	Wire.begin(); //se pornesc toate instantele initializate mai sus
 
    // I2c adresa default este 0x76, daca este diferita, se modifica bme.begin(Addr)
    if (!bme.begin(0x76)) {
        Serial.println("Nu s-a gasit niciun senzor la adresa asta!");
        while (1);
    }
    
    Serial.println("-- BME280 METEO --");
	lcd.begin(20, 4); //Initializare LCD2004
	lcd.createChar(0, grad); // Creare simbol grad
//	lcd.createChar(1, arrow_d); // Creare simbol sageata jos
	lcd.createChar(2, pic_sus); // Creare simbol picatura sus
	lcd.createChar(3, pic_jos); // Creare simbol picatura jos
	lcd.createChar(4, soare_st); // Creare simbol soare_st
	lcd.createChar(5, soare_dr); // Creare simbol soare_dr
  lcd.createChar(6, nor_st); // Creare simbol nor_st
  lcd.createChar(7, nor_dr); // Creare simbol nor_dr
	
	//Printare mesaj logo pe LCD
    lcd.setCursor(5, 0);
    lcd.print("Pekatonix");
    lcd.setCursor(0, 1);
    lcd.print("creat : Peter Gyorfi");
    lcd.setCursor(0, 2);
    lcd.print("varianta 4");
	  lcd.setCursor(0, 3);
    lcd.print("pekatonix@gmail.com");
    delay(2500);
        lcd.clear(); //Stergem ecranul...
}


void loop() { 
  tempint = bme.temperatureValue();
  pa = bme.pressureValue();
  hum = bme.humidityValue();
  alt = bme.altitudeValue(SEA_LEVEL_PRESSURE);
  
  //Afisare temperatura interioara pe monitor
	Serial.print("Temp:");
	Serial.print(tempint);
	Serial.println(" C");
  //Afisare temperatura interioara pe LCD
        lcd.setCursor(0, 0);
        lcd.print("Temp int:");
        lcd.setCursor(16, 0);
        lcd.write(byte(0));
        lcd.print("C"); 
        lcd.setCursor(10, 0);
        lcd.print(tempint); 
  
  // Afisare presiune atmosferica, in hPa, pe monitor
	Serial.print("Pa:");
	Serial.print(pa /100);
	Serial.println(" hPa");
  // Afisare presiune atmosferica, in hPa, pe LCD
		lcd.setCursor(0, 2);
		lcd.print("Pres: ");
		lcd.setCursor(13, 2);
		lcd.print("hPa");
		lcd.setCursor(6, 2);
		lcd.print(pa /100);
	
  //Afisare umiditate pe monitor
	Serial.print("Hum:");
	Serial.print(hum);
	Serial.println(" %");
  //Afisare umiditate pe LCD
        lcd.setCursor(0, 3);
        lcd.print("Umiditate:");
        lcd.setCursor(16,3);
        lcd.print(" %");
        lcd.setCursor(11,3);
        lcd.print(hum); 
 
  //Afisare altitudine pe monitor
	Serial.print("Altitudine:");
	Serial.print(alt);
	Serial.println(" m");
	
  //Afisare temperatura exterioara pe LCD 
        senzor.requestTemperatures(); //cerem temperaturile de la ds18b20
        tempext = senzor.getTempCByIndex(0);  //temperatura exterioara cu DS18B20
		    lcd.setCursor(0, 1);                 
		    lcd.print("Temp ext: ");	
        lcd.setCursor(16, 1);
        lcd.write(byte(0));
        lcd.print("C"); 
        lcd.setCursor(10, 1);
		    lcd.print(tempext);   
    
 /*
  //Afisare altitudine pe LCD
        lcd.setCursor(0, 3);
        lcd.print("Altitudine:");
        lcd.setCursor(18,3);
        lcd.print(" m");
        lcd.setCursor(12,3);
        lcd.print(alt); 
  */  

  delay(5000);
  
//Afisare picaturi de ploaie la presiunea atmosferica scazuta
  if (pa /100<=969)  // 978.46 hPa este presiunea normala altitudinea de 320m
		{
			lcd.setCursor(18,2);
			lcd.write(byte(2));
			lcd.setCursor(19,2);
			lcd.write(byte(3));
		}

//Afisare soare la presiunea atmosferica ridicata
  else if (pa /100>=978.46)   
   {
      lcd.setCursor(18,2);
      lcd.write(byte(4));
      lcd.setCursor(19,2);
      lcd.write(byte(5));
  }

//Afisare nor la presiunea atmosferica intre 969-978.46 hPa
   else // (pa /100>969 || <978.46)
   {
      lcd.setCursor(18,2);
      lcd.write(byte(6));
      lcd.setCursor(19,2);
      lcd.write(byte(7));
  } 
}
	
  
