/*!
 * Creat de Peter Gyorfi
 * Statie meteo cu senzor BME280 si DS18B20  pentru temperatura exterioara
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
	//Printare mesaj logo pe LCD
    lcd.setCursor(5, 0);
    lcd.print("Pekatonix");
    lcd.setCursor(0, 1);
    lcd.print("creat : Peter Gyorfi");
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
  
  //Display inside temperature on console
	Serial.print("Temp:");
	Serial.print(tempint);
	Serial.println(" C");
  //Afisare temperatura interioara pe LCD
        lcd.setCursor(0, 0);
        lcd.print("Temp int:");
        lcd.setCursor(16, 0);
        lcd.print((char)223);
        lcd.print("C"); //We do it this way so that The "C" is always at the same place on the screen
        lcd.setCursor(10, 0);
        lcd.print(tempint); //Some of the digits will overwrite the spaces left before "C"
  
  // Display atmospheric pressue in hPa in console
	Serial.print("Pa:");
	Serial.print(pa /100);
	Serial.println(" hPa");
  // Display atmospheric pressure in hPa on LCD
		lcd.setCursor(0, 2);
		lcd.print("Pres atm: ");
		lcd.setCursor(17, 2);
		lcd.print("hPa");
		lcd.setCursor(10, 2);
		lcd.print(pa /100);
	
  //Display Humidity in console
	Serial.print("Hum:");
	Serial.print(hum);
	Serial.println(" %");
  //Display Humidity on LCD
        lcd.setCursor(0, 3);
        lcd.print("Umiditate:");
        lcd.setCursor(16,3);
        lcd.print(" %");
        lcd.setCursor(11,3);
        lcd.print(hum); 
 
  //Display Altitude in console
	Serial.print("Alt:");
	Serial.print(alt);
	Serial.println(" m");
	
  //Afisare temperatura exterioara pe LCD 
        senzor.requestTemperatures(); //cerem temperaturile de la ds18b20
        tempext = senzor.getTempCByIndex(0);  //temperatura exterioara cu DS18B20
		    lcd.setCursor(0, 1);                 
		    lcd.print("Temp ext: ");	
        lcd.setCursor(16, 1);
        lcd.print((char)223);
        lcd.print("C"); //We do it this way so that The "C" is always at the same place on the screen
        lcd.setCursor(10, 1);
		    lcd.print(tempext);                          
 /*
  //Display Altitude on LCD
        lcd.setCursor(0, 3);
        lcd.print("Altitudine:");
        lcd.setCursor(18,3);
        lcd.print(" m");
        lcd.setCursor(12,3);
        lcd.print(alt); 
  */  
  delay(5000);
}
