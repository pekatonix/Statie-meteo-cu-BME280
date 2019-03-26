
/***************************************************************************
  Example for BME280 Weather Station
  written by Thiago Barros for BlueDot UG (haftungsbeschränkt)
  BSD License

  This sketch was written for the Bosch Sensor BME280.
  The BME280 is a MEMS device for measuring temperature, humidity and atmospheric pressure.
  For more technical information on the BME280, please go to ------> http://www.bluedot.space
------------------------------------------------------------------------------------------------
 Dawn & Dusk controller. http://andydoz.blogspot.ro/2014_08_01_archive.html
 16th August 2014.
 (C) A.G.Doswell 2014
 adapted sketch by niq_ro from http://nicuflorica.blogspot.ro & http://arduinotehniq.blogspot.com/
 Date and time functions using a DS1307 RTC connected via I2C and Wire lib

 Designed to control a relay connected to pin A3. Pin goes low during daylight hours and high during night. Relay uses active low, so is
 "On" during the day. This is connected to the fountain pump in my garden.

 Time is set using a rotary encoder with integral push button. The Encoder is connected to interrupt pins D2 & D3 (and GND), 
 and the push button to pin analogue 0 (and GND)
 The RTC is connections are: Analogue pin 4 to SDA. Connect analogue pin 5 to SCL.
 A 4 x 20 LCD display is connected as follows (NOTE. This is NOT conventional, as interrupt pins are required for the encoder)
      Arduino LCD  
      D7      DB7
      D6      DB6
      D5      DB5
      D4      DB4
      D8      RS
      D9      E
 
 Use: Pressing and holding the button will enter the clock set mode (on release of the button). Clock is set using the rotary encoder. 
 The clock must be set to UTC.
 Pressing and releasing the button quickly will display the current sun rise and sun set times. Pressing the button again will enter the mode select menu. 
 Modes are AUTO: On when the sun rises, off when it sets.
           ON: Permanently ON
           OFF: Permanently OFF (Who'd have guessed it?)

 Schimbati LATITUDINEA si LONGITUDINEA functie de locatia dv.
-------------------------------------------------------------------------------------------------------------------------
  Sketch adaptat de Peter Gyorfi 
  Statie meteo cu senzor BME280
  Varianta 5 - schimbata libraria si modificat algoritmul  de masura, adaugat ceas astronomic
  @copyright GNU General Public License
  Tg-Mures, 26.03.2019.
 ***************************************************************************/


#include <Wire.h>
#include <avr/wdt.h>
#include "BlueDot_BME280.h"
#include <LiquidCrystal.h>
#include "RTClib.h" // from https://github.com/adafruit/RTClib
#include <Encoder.h> // from http://www.pjrc.com/teensy/td_libs_Encoder.html
#include <TimeLord.h> // from http://swfltek.com/arduino/timelord.html. When adding it to your IDE, rename the file, removing the "-depreciated" 
// http://www.timeanddate.com/sun/romania/craiova - for control sun time
BlueDot_BME280 bme280 = BlueDot_BME280();
RTC_DS1307 RTC; // Tells the RTC library that we're using a DS1307 RTC
Encoder knob(2, 3); //encoder connected to pins 2 and 3 (and ground)

//the variables provide the holding values for the set clock routine
int setyeartemp; 
int setmonthtemp;
int setdaytemp;
int sethourstemp;
int setminstemp;
int setsecs = 0;
int maxday; // maximum number of days in the given month
int TimeMins; // number of seconds since midnight
int TimerMode = 2; //mode 0=Off 1=On 2=Auto
int TimeOut = 10;
int TimeOutCounter;

// These variables are for the push button routine
int buttonstate = 0; //flag to see if the button has been pressed, used internal on the subroutine only
int pushlengthset = 3000; // value for a long push in mS
int pushlength = pushlengthset; // set default pushlength
int pushstart = 0;// sets default push value for the button going low
int pushstop = 0;// sets the default value for when the button goes back high

int knobval; // value for the rotation of the knob
boolean buttonflag = false; // default value for the button flag


//const int TIMEZONE = 0; //UTC
const int TIMEZONE = 2; //UTC Targu Mures (Romania) - http://www.worldtimebuddy.com/utc-to-romania-craiova
//const float LATITUDE = 51.89, LONGITUDE = -2.04; // set YOUR position here 
const float LATITUDE =  46.33, LONGITUDE = 24.34; // Craiova GPS position
int Sunrise, Sunset; //sunrise and sunset expressed as minute of day (0-1439)
TimeLord myLord; // TimeLord Object, Global variable
byte sunTime[]  = {0, 0, 0, 1, 1, 13}; // 17 Oct 2013
int SunriseHour, SunriseMin, SunsetHour, SunsetMin; //Variables used to make a decent display of our sunset and sunrise time.

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

byte arrow_u[8] = {   //Simbol sageata sus
	B00100, 
	B01110, 
	B11111, 
	B00100, 
	B00100, 
	B00100, 
	B00100, 
	B00100 
}; 

float pa, hum, alt;
float temp;

void setup() {
    Serial.begin(9600);
    Wire.begin(); //start I2C interface
    RTC.begin(); //start RTC interface
	
  Serial.println(F("Basic Weather Station"));

    bme280.parameter.communication = 0;                  // Alegem protocolul de comunicatie. Am eliminat din cod secventa pentru SPI
  
    bme280.parameter.I2CAddress = 0x76;                  // Adresa senzorului (0x76 sau 0x77)

    bme280.parameter.sensorMode = 0b11;                   //Choose sensor mode
    bme280.parameter.IIRfilter = 0b100;                    //Setup for IIR Filter      
    bme280.parameter.humidOversampling = 0b101;            //Setup Humidity Oversampling
    bme280.parameter.tempOversampling = 0b101;             //Setup Temperature Ovesampling
    bme280.parameter.pressOversampling = 0b101;            //Setup Pressure Oversampling 
    bme280.parameter.pressureSeaLevel = 1013.25;           //default value of 1013.25 hPa
    bme280.parameter.tempOutsideCelsius = 15;              //default value of 15°C

 
  if (bme280.init() != 0x60)
  {    
                                                         //If the Arduino fails to identify the BME280, program stops and shows the Troubleshooting guide
    wdt_disable();                                       //The Watchdog Timer is turned off
    
    
    Serial.println(F("Ops! BME280 could not be found!"));
    Serial.println(F("Please check your connections."));
    Serial.println();
    Serial.println(F("Troubleshooting Guide"));
    Serial.println(F("*************************************************************"));
    Serial.println(F("1. Let's check the basics: Are the VCC and GND pins connected correctly? If the BME280 is getting really hot, then the wires are crossed."));
    Serial.println();
    Serial.println(F("2. Are you using the I2C mode? Did you connect the SDI pin from your BME280 to the SDA line from the Arduino?"));
    Serial.println();
    Serial.println(F("3. And did you connect the SCK pin from the BME280 to the SCL line from your Arduino?"));
    Serial.println();
    Serial.println(F("4. Are you using the alternative I2C Address(0x76)? Did you remember to connect the SDO pin to GND?"));
    Serial.println();
    Serial.println(F("5. If you are using the default I2C Address (0x77), did you remember to leave the SDO pin unconnected?"));
    Serial.println();
    Serial.println(F("6. Are you using the SPI mode? Did you connect the Chip Select (CS) pin to the pin 10 of your Arduino (or to wherever pin you choosed)?"));
    Serial.println();
    Serial.println(F("7. Did you connect the SDI pin from the BME280 to the MOSI pin from your Arduino?"));
    Serial.println();
    Serial.println(F("8. Did you connect the SDO pin from the BME280 to the MISO pin from your Arduino?"));
    Serial.println();
    Serial.println(F("9. And finally, did you connect the SCK pin from the BME280 to the SCK pin from your Arduino?"));
    Serial.println();
    
    while(1);
  }

  else
  {
    Serial.println(F("BME280 detected!"));
  }
  Serial.println();
  Serial.println();

	lcd.begin(20, 4); //Initializare LCD2004
	lcd.createChar(0, grad); // Creare simbol grad
	lcd.createChar(1, arrow_d); // Creare simbol sageata jos
	lcd.createChar(2, arrow_u); // Creare simbol sageata sus
 
	//Printare mesaj logo pe LCD
    lcd.setCursor(5, 0);
    lcd.print("Pekatonix");
    lcd.setCursor(0, 1);
    lcd.print("creat : Peter Gyorfi");
    lcd.setCursor(0, 2);
    lcd.print("varianta 5");
	lcd.setCursor(0, 3);
    lcd.print("pekatonix@gmail.com");
    delay(2500);
    lcd.clear(); //Stergem ecranul...
		
	pinMode(A0,INPUT);//push button on encoder connected to A0 (and GND)
    digitalWrite(A0,HIGH); //Pull A0 high	
//	pinMode(A3,OUTPUT); //Relay connected to A3
//  digitalWrite (A3, HIGH); //sets relay off (default condition)

    //Checks to see if the RTC is runnning, and if not, sets the time to the time this sketch was compiled.
    if (! RTC.isrunning()) {
    RTC.adjust(DateTime(__DATE__, __TIME__));
	}
 
     //Timelord initialisation
    myLord.TimeZone(TIMEZONE * 60);
    myLord.Position(LATITUDE, LONGITUDE);
    CalcSun ();
}

void loop() 
{ 
   wdt_reset();       // Functia reseteaza conrtorul Watchdog Timer. Utilizati intotdeauna aceasta functie daca Watchdog Timer este pornit.

	temp = bme280.readTempC();
	hum = bme280.readHumidity();
	pa = bme280.readPressure();
	alt = bme280.readAltitudeMeter();
	
	// Afisare date pe monitor serial
   Serial.print(F("Duration in Seconds:\t\t"));
   Serial.println(float(millis())/1000);
 
   Serial.print(F("Temperature in Celsius:\t\t")); 
   Serial.println(temp);
     
   Serial.print(F("Humidity in %:\t\t\t")); 
   Serial.println(hum);

   Serial.print(F("Pressure in hPa:\t\t")); 
   Serial.println(pa);

   Serial.print(F("Altitude in Meters:\t\t")); 
   Serial.println(alt);
   
   Serial.println();
   Serial.println();
   
	// Afisare date pe LCD
	DateTime now = RTC.now(); //get time from RTC
 
    //    Display current time
    lcd.setCursor (10,0);
    lcd.print(now.day(), DEC);
    lcd.print('/');
    lcd.print(now.month());
    lcd.print('/');
    lcd.print(now.year(), DEC);
    lcd.print(" ");
    lcd.setCursor (0,0);
    lcd.print(now.hour(), DEC);
    lcd.print(':');
    if (now.minute() <10) 
      {
        lcd.print("0");
      }
    lcd.print(now.minute(), DEC);
    lcd.print(':');
    if (now.second() <10) 
      {
        lcd.print("0");
      }
    lcd.print(now.second());
//    lcd.print("     ");
	
        lcd.setCursor(0, 1);  // Afisare temperatura exterioara
        lcd.print("Temp ext: ");
        lcd.setCursor(17, 1);
        lcd.write(byte(0));
        lcd.print("C"); 
        lcd.setCursor(11, 1);
        lcd.print(temp);	
		
		lcd.setCursor(0, 2);  // Afisare presiune atmosferica
		lcd.print("Pres: ");
		lcd.setCursor(13, 2);
		lcd.print("hPa");
		lcd.setCursor(6, 2);
		lcd.print(pa);		
		
        lcd.setCursor(0, 3);  // Afisare umiditate
        lcd.print("Umiditate:");
        lcd.setCursor(16,3);
        lcd.print(" %");
        lcd.setCursor(11,3);
        lcd.print(hum);		
	/*	
        lcd.setCursor(0, 3);  // Afisare altitudine
        lcd.print("Altitudine:");
        lcd.setCursor(18,3);
        lcd.print(" m");
        lcd.setCursor(12,3);
        lcd.print(alt);		
	*/
   delay(1000);  


 //Afisare sageti up&down functie de presiunea atmosferica
		if (pa<978.46)  // 978.21 hPa este presiunea normala altitudinea de 320m
		{
			lcd.setCursor(18,2);
			lcd.write(byte(1));
			lcd.setCursor(19,2);
			lcd.write(byte(1));
		}
		else if (pa>=978.46)
   {
			lcd.setCursor(18,2);
			lcd.write(byte(2));
			lcd.setCursor(19,2);
			lcd.write(byte(2));
  }

  //current time in minutes since midnight (used to check against sunrise/sunset easily)
    TimeMins = (now.hour() * 60) + now.minute();
    
    // Calculate sun times once a day at a minute past midnight
    if (TimeMins == 1) {
      CalcSun ();
    }
    if (TimerMode ==2) {
      if (TimeMins >= Sunrise && TimeMins <=Sunset-1) { //If it's after sunrise and before sunset, switch our relay on
          digitalWrite (A3, LOW);
          lcd.setCursor (9,1);
          lcd.print ("*");
        }
        else {  //otherwise switch it off
          digitalWrite (A3, HIGH);
          lcd.setCursor (9,1);
          lcd.print ("!");
        }
      }
       if (TimerMode ==0) {
         digitalWrite (A3, HIGH);
         lcd.setCursor (9,1);
         lcd.print ("!");
       }
     
       if (TimerMode ==1) {
         digitalWrite (A3, LOW);
         lcd.setCursor (9,1);
         lcd.print ("*");
       }
    
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    delay (10);
    
    if (pushlength <pushlengthset) {
     
      ShortPush ();   
    }
    
       
       //This runs the setclock routine if the knob is pushed for a long time
       if (pushlength >pushlengthset) {
         lcd.clear();
         DateTime now = RTC.now();
         setyeartemp=now.year(),DEC;
         setmonthtemp=now.month(),DEC;
         setdaytemp=now.day(),DEC;
         sethourstemp=now.hour(),DEC;
         setminstemp=now.minute(),DEC;
         setclock();
         pushlength = pushlengthset;
       };
}

//sets the clock
void setclock (){
   setyear ();
   lcd.clear ();
   setmonth ();
   lcd.clear ();
   setday ();
   lcd.clear ();
   sethours ();
   lcd.clear ();
   setmins ();
   lcd.clear();
   
   RTC.adjust(DateTime(setyeartemp,setmonthtemp,setdaytemp,sethourstemp,setminstemp,setsecs));
   CalcSun ();
   delay (1000);
   
}

// subroutine to return the length of the button push.
int getpushlength () {
  buttonstate = digitalRead(A0);  
       if(buttonstate == LOW && buttonflag==false) {     
              pushstart = millis();
              buttonflag = true;
          };
          
       if (buttonstate == HIGH && buttonflag==true) {
         pushstop = millis ();
         pushlength = pushstop - pushstart;
         buttonflag = false;
       };
       return pushlength;
}
// The following subroutines set the individual clock parameters
int setyear () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Setare An");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setyeartemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) { //bit of software de-bounce
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setyeartemp=setyeartemp + knobval;
    if (setyeartemp < 2014) { //Year can't be older than currently, it's not a time machine.
      setyeartemp = 2014;
    }
    lcd.print (setyeartemp);
    lcd.print("  "); 
    setyear();
}
  
int setmonth () {
//lcd.clear();
   lcd.setCursor (0,0);
    lcd.print ("Setare Luna");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setmonthtemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setmonthtemp=setmonthtemp + knobval;
    if (setmonthtemp < 1) {// month must be between 1 and 12
      setmonthtemp = 1;
    }
    if (setmonthtemp > 12) {
      setmonthtemp=12;
    }
    lcd.print (setmonthtemp);
    lcd.print("  "); 
    setmonth();
}

int setday () {
  if (setmonthtemp == 4 || setmonthtemp == 5 || setmonthtemp == 9 || setmonthtemp == 11) { //30 days hath September, April June and November
    maxday = 30;
  }
  else {
  maxday = 31; //... all the others have 31
  }
  if (setmonthtemp ==2 && setyeartemp % 4 ==0) { //... Except February alone, and that has 28 days clear, and 29 in a leap year.
    maxday = 29;
  }
  if (setmonthtemp ==2 && setyeartemp % 4 !=0) {
    maxday = 28;
  }
//lcd.clear();  
   lcd.setCursor (0,0);
    lcd.print ("Setare zi");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setdaytemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setdaytemp=setdaytemp+ knobval;
    if (setdaytemp < 1) {
      setdaytemp = 1;
    }
    if (setdaytemp > maxday) {
      setdaytemp = maxday;
    }
    lcd.print (setdaytemp);
    lcd.print("  "); 
    setday();
}

int sethours () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Setare Ora");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return sethourstemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    sethourstemp=sethourstemp + knobval;
    if (sethourstemp < 1) {
      sethourstemp = 1;
    }
    if (sethourstemp > 23) {
      sethourstemp=23;
    }
    lcd.print (sethourstemp);
    lcd.print("  "); 
    sethours();
}

int setmins () {
//lcd.clear();
   lcd.setCursor (0,0);
    lcd.print ("Setare Minute");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setminstemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setminstemp=setminstemp + knobval;
    if (setminstemp < 0) {
      setminstemp = 0;
    }
    if (setminstemp > 59) {
      setminstemp=59;
    }
    lcd.print (setminstemp);
    lcd.print("  "); 
    setmins();
}

int setmode () { //Sets the mode of the timer. Auto, On or Off
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Set Mode");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return TimerMode;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    TimerMode=TimerMode + knobval;
    if (TimerMode < 0) {
      TimerMode = 0;
    }
    if (TimerMode > 2) {
      TimerMode=2;
    }
    if (TimerMode == 0) {
    lcd.print("Off (!)");
    lcd.print("  "); 
    }
    if (TimerMode == 1) {
    lcd.print("On (*)");
    lcd.print("  "); 
    }
    if (TimerMode == 2) {
    lcd.print("Auto");
    lcd.print("  "); 
    }
    setmode ();
}

int CalcSun () { //Calculates the Sunrise and Sunset times
    DateTime now = RTC.now();
    sunTime[3] = now.day(); // Give Timelord the current date
    sunTime[4] = now.month();
    sunTime[5] = now.year();
    myLord.SunRise(sunTime); // Computes Sun Rise.
    Sunrise = sunTime[2] * 60 + sunTime[1]; // Sunrise returned in minutes past midnight
    SunriseHour = sunTime[2];
    SunriseMin = sunTime [1];
    sunTime[3] = now.day(); // Uses the Time library to give Timelord the current date
    sunTime[4] = now.month();
    sunTime[5] = now.year();
    myLord.SunSet(sunTime); // Computes Sun Set.
    Sunset = sunTime[2] * 60 + sunTime[1]; // Sunset returned in minutes past midnight
    SunsetHour = sunTime[2];
    SunsetMin = sunTime [1];
}

void ShortPush () {
  //This displays the calculated sunrise and sunset times when the knob is pushed for a short time.
lcd.clear();
for (long Counter = 0; Counter < 604 ; Counter ++) { //returns to the main loop if it's been run 604 times 
                                                     //(don't ask me why I've set 604,it seemed like a good number)
  lcd.setCursor (0,0);
  lcd.print ("Soare rasare ");
  lcd.print (SunriseHour);
  lcd.print (":");
  if (SunriseMin <10) 
     {
     lcd.print("0");
     }
  lcd.print (SunriseMin);
  lcd.setCursor (0,2);
  lcd.print ("Soare apune ");
  lcd.print (SunsetHour);
  lcd.print (":"); 
    if (SunsetMin <10) 
     {
     lcd.print("0");
     }
  lcd.print (SunsetMin);        

    
  //If the knob is pushed again, enter the mode set menu
  pushlength = pushlengthset;
  pushlength = getpushlength ();
  if (pushlength != pushlengthset) {
    lcd.clear ();
    TimerMode = setmode ();

  }
  
}
lcd.clear();
}
