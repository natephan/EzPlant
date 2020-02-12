/* analyzer Arduino
  This Arduino utilizes the DHT11 sensor to read the room's temperature
  and humidity. A soil moisture sensor is also used to read the moisture
  in soil. These values are displayed on an LCD screen. The values are sent
  to the master Arduino every 10 minutes. The master Arduino will send
  minimum and maximum parameters for the temperature, humidity, and moisture
  to this Arduino. This Arduino will use those parameters to determine
  if the current values are within the proper range. If they are not,
  then a warning will be displayed.
*/

#include <LiquidCrystal.h> // library for lcd
#include "DHT.h" // library to make the sensor work
#include <Wire.h> // communication library
#define DHTPIN A0 // dht11 pin
#define DHTTYPE DHT11 //type of our dht sensor

// set up LCD
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// set up DHT11 sensor
DHT dht(DHTPIN, DHTTYPE);

// set pin for moisture sensor
int moistPin = A1;

// initialize variables
int curHum, curTemp, curMoist;
int minTemp, maxTemp, minHum, maxHum, minMoist, maxMoist;
int started = 0;
int master = 0;
int analyzer = 1;

// array of min/max parameters sent from master
int minMaxes[6];

void setup() {
  Serial.begin(9600); // start Serial for debugging
  Wire.begin(analyzer); // start Wire address for analyzer (1)
  Wire.onRequest(sendInfo); // when data is requested, call sendInfo
  Wire.onReceive(getInfo); // when data is received, call getInfo
  lcd.begin(16, 2); // setting up the lcd
  dht.begin(); // setting up the dht
}

void loop() {
  // get temperature and humidity
  curHum = (int) dht.readHumidity();
  curTemp = (int) dht.readTemperature(true);
  
  // display temp and humidity on LCD screen
  lcd.setCursor(0, 0);
  lcd.print ("T: ");
  lcd.print (curTemp);
  lcd.print ("*F ");
  lcd.print ("H: ");
  lcd.print (curHum);
  lcd.print ("%");
  
  // get moisture values from sensor
  curMoist = analogRead(moistPin);
  
  // map moisture values to 0-100 from 320-620 for better readability
  curMoist = map(curMoist, 320, 620, 100, 0);
  
  // display moisture value on LCD screen
  lcd.setCursor(0, 1);
  lcd.print("M: ");
  lcd.print(curMoist);
  
  // if the min/max parameters have been received from the master
  if (started == 1)
  {
    // if curTemp is lower than minimum, display warning
    if (curTemp < minTemp)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Freezing!");
      lcd.setCursor(0, 1);
      lcd.print(curTemp);
    }
    
    // if curTemp is higher than maximum, display warning
    else if (curTemp > maxTemp)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Burning!");
      lcd.setCursor(0, 1);
      lcd.print(curTemp);
    }
    
    // if curHum is lower than minimum, display warning
    else if (curHum < minHum)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Hum too Low!");
      lcd.setCursor(0, 1);
      lcd.print(curHum);
    }
    
    // if curHum is higher than maximum, display warning
    else if (curHum > maxHum)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Hum too High!");
      lcd.setCursor(0, 1);
      lcd.print(curHum);
    }
    
    // if curMoist is lower than minimum, display warning
    else if (curMoist < minMoist)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("More Water!");
      lcd.setCursor(0, 1);
      lcd.print(curMoist);
    }
    
    // if curTemp is higher than maximum, display warning
    else if (curMoist > maxMoist)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Drowning!");
      lcd.setCursor(0, 1);
      lcd.print(curMoist);
    }
  }
  delay(500); // delay for LCD readability
  lcd.clear(); // clear LCD display
}


// called when a request is made from master, sends current sensor readings
void sendInfo()
{
  Wire.write(curTemp);
  Wire.write(curHum);
  Wire.write(curMoist);
}


// called when info is received from the master, sets the min/max variables stored in master
void getInfo(int howMuch)
{
  // populate array with min/max parameters
  for (int i = 0; i < 6; i++)
  {
    if (Wire.available())
    {
      minMaxes[i] = Wire.read();
    }
  }
  
  // set local min/max variables to values sent from master
  minTemp = minMaxes[0];
  maxTemp = minMaxes[1];
  minHum = minMaxes[2];
  maxHum = minMaxes[3];
  minMoist = minMaxes[4];
  maxMoist = minMaxes[5];
  
  // Serial output for debugging
  Serial.println(minTemp);
  Serial.println(maxTemp);
  Serial.println(minHum);
  Serial.println(maxHum);
  Serial.println(minMoist);
  Serial.println(maxMoist);
  
  // set started to 1, will enable the parameter checking conditions
  started = 1;
}
