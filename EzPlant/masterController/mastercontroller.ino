/* master arduino
  This Arduino is the master Arduino and controls the other 2 Arduinos.
  Parameters are set in Serial by the user in this Arduino. Those parameters
  are sent to the analyzer Arduino for condition checking. The current values
  from sensors are sent from the analyzer to this Arduino, where the current
  values are checked. If the values are not within the proper range, points are
  subtracted. If they are in proper range, points are added. The current time and
  points are displayed on an LCD screen attached to this Arduino. This Arduino
  also sends commands to the performer Arduino. These commands turn on or off the
  light attached to the performer Arduino. There is also a button on this Arduino
  that allows the resetting of the parameters.

  Team 34
  Nathan Phan - nphan23
  Ali S Baig - abaig28
  Mohab Mustafa - mmustaf2
  EzPlant
  We will be using multiple Arduinos to control a plant care system which monitors the
  plant's environmental conditions, soil moisture,
  and controls a light. Three Arduinos will communicate with each other and toggle the
  light based on time and send sensor readings and
  plant parameters, which will be stored in a master Arduino and will be set by the user.
  These sensors will measure humidity, temperature,
  and soil moisture. The environment conditions are used to create a game, where points
  are received if the plant is being properly cared for and
  points are detracted when conditions are not met.
*/

#include <LiquidCrystal.h>
#include <Wire.h>
#include <Time.h>
#include <TimeLib.h>

// button for parameter reset
int buttonPin = 7;

// initialize pins for LCD
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// initialize variables
int curHour, curMin, curSec, lastSec;
int minMoist, maxMoist, curMoist;
int minTemp, maxTemp, curTemp;
int minHum, maxHum, curHum;
int lastRead, curRead = 0;
int buttonState, checkOne, checkTwo;
int onHour, offHour;
int points = 0;

int analyzeVals[3]; // array for receiving current values from the analyzer
int minMaxes[6]; // array for sending min/max parameters to the analyzer

// commands for operator
char lightOn = 'O';
char lightOff = 'F';

// command for analyzer
char getMoisture = 'M';

// address of arduinos
int master = 0;
int analyzer = 1;
int performer = 2;


void setup() {
  Serial.begin(9600);
  
  // set master address
  Wire.begin(master);
  
  // set up lcd screen
  lcd.begin(16, 2);
  
  // set up button
  pinMode(buttonPin, INPUT);
  buttonState = digitalRead(buttonPin);
  
  // set up plant parameters
  setParameters();
}


void loop() {
  // displays current time
  lcdDisplay();
  
  // checks for button bounce
  checkOne = digitalRead(buttonPin);
  delay(10);
  checkTwo = digitalRead(buttonPin);
  if (checkOne == checkTwo)
  {
    if (checkOne != buttonState)
    {
      // if button was pressed, allow reset of parameters
      if (checkOne == HIGH)
      {
        setParameters();
      }
    }
  }
  // assign buttonState to last state to prevent counting button holds
  buttonState = checkOne;
  
  // every ten minutes, request moisture, temperature, and humidity values from analyzer slave
  // give or detract points depending on current values
  // repeat every 10 minutes
  curRead = minute();
  if (lastRead > curRead)
  {
    curRead += 60;
  }
  curRead -= lastRead;
  
  // if 10 minutes has passed since last read
  if (curRead == 10)
  {
    // set time to current time to reset counter
    lastRead = minute();
    
    // request current moisture readings from analyzer
    requestData(analyzeVals, analyzer);
    
    // set current values to values read from the analyzer
    curTemp = analyzeVals[0];
    curHum = analyzeVals[1];
    curMoist = analyzeVals[2];
    
    // if the plant's moisture is not within range, subtract 10 points
    if (curMoist < minMoist || curMoist > maxMoist)
    {
      points -= 10;
    }
    
    // if plant is watered properly, add 50 points
    else
    {
      points += 50;
    }
    
    // if environment's temparture is not within range, subtract 5 points
    if (curTemp < minTemp || curTemp > maxTemp)
    {
      points -= 5;
    }
    
    // if the temperature is acceptable, add 25 points
    else
    {
      points += 25;
    }
    
    // if environment's humidity is not within range, subtract 5 points
    if (curHum < minHum || curHum > maxHum)
    {
      points -= 5;
    }
    
    // if humidity is acceptable, add 25 points
    else
    {
      points += 25;
    }
  }
  
  // if current time is between time to turn on and time to turn off
  if (hour() >= onHour && hour() < offHour)
  {
    Serial.println("TURN ON");
    
    // send command to performer to turn light on
    sendCommand(lightOn, performer);
  }
  
  // if current time is after time to turn off or before time to turn on
  if (hour() >= offHour || hour() < onHour)
  {
    Serial.println("TURN OFF");
    
    // send command to performer to turn light off
    sendCommand(lightOff, performer);
  }
  lcd.clear();
}


// displays the current time on the lcd screen
void lcdDisplay()
{
  // print current time
  lcd.setCursor(0, 0);
  lcd.print(hour());
  lcd.print(":");
  lcd.print(minute());
  lcd.print(":");
  if (second() < 10)
  {
    lcd.print("0");
  }
  lcd.print(second());
  lcd.setCursor(0, 1);
  lcd.print("Points: ");
  lcd.print(points);
}


// clears Wire buffer
void clearWire()
{
  while (Wire.available())
  {
    Wire.read();
  }
}


// clears Serial buffer
void clearSerial()
{
  while (Serial.available())
  {
    Serial.read();
  }
}


// reads input from Serial
void getInput(int& data)
{
  // wait for serial input
  while (!Serial.available()) {}
  
  // get input from Serial as an int
  data = Serial.parseInt();
  
  // clear Serial input buffer by reading the rest of the buffer
  clearSerial();
}


// request data from target
void requestData(int data[], int target)
{
  Wire.requestFrom(target, 3);
  for (int i = 0; i < 3; i++)
  {
    if (Wire.available())
    {
      analyzeVals[i] = Wire.read();
    }
  }
  clearWire();
}


// sends data to analyzer arduino
void sendCommand(char command, int target)
{
  Wire.beginTransmission(target); // transmit to target arduino
  Wire.write(command); // sends five bytes
  Wire.endTransmission(); // stop transmitting
  clearWire(); // stop transmitting
}


// sends data to analyzer arduino
void sendAnalyze(int data[])
{
  Wire.beginTransmission(analyzer); // transmit to analyzer
  for (int i = 0; i < 6; i++)
  {
    Wire.write(data[i]); // sends data
  }
  Wire.endTransmission(); // stop transmitting
  clearWire(); // stop transmitting
}


// allows setting of plant parameters
void setParameters()
{
  Serial.println("Set paramaters. Mins must be less than maxes.");
  // enter time to turn light on
  Serial.println("Enter Hour to Turn On Light (0-22): ");
  getInput(onHour);
  // checks if input is in correct interval
  while (onHour < 0 || onHour > 22)
  {
    Serial.println("Enter Hour to Turn On Light (0-22): ");
    // if input was wrong, repeat loop until correct
    getInput(onHour);
  }
  Serial.println(onHour);
  
  // enter time to turn light off
  Serial.print("Enter Hour to Turn Off Light (");
  Serial.print(onHour + 1);
  Serial.println("-23): ");
  getInput(offHour);
  // checks if input is in correct interval
  while (offHour <= onHour || offHour > 23)
  {
    Serial.print("Enter Hour to Turn Off Light (");
    Serial.print(onHour + 1);
    Serial.println("-23): ");
    // if input was wrong, repeat loop until correct
    getInput(offHour);
  }
  Serial.println(offHour);
  
  // enter minimum temperature
  Serial.println("Enter Minimum Temperature (0-99): ");
  getInput(minTemp);
  // checks if input is in correct interval
  while (minTemp < 0 || minTemp > 99)
  {
    Serial.println("Enter Minimum Temperature (0-99): ");
    // if input was wrong, repeat loop until correct
    getInput(minTemp);
  }
  Serial.println(minTemp);
  
  // enter maximum temperature
  Serial.print("Enter Maximum Temperature (");
  Serial.print(minTemp + 1);
  Serial.println("-100): ");
  getInput(maxTemp);
  // checks if input is in correct interval
  while (maxTemp <= minTemp || maxTemp > 100)
  {
    Serial.print("Enter Maximum Temperature (");
    Serial.print(minTemp + 1);
    Serial.println("-100): ");
    // if input was wrong, repeat loop until correct
    getInput(maxTemp);
  }
  Serial.println(maxTemp);
  
  // enter minimum humidity
  Serial.println("Enter Minimum Humidity (0-99): ");
  getInput(minHum);
  // checks if input is in correct interval
  while (minHum < 0 || minHum > 99)
  {
    Serial.println("Enter Minimum Humidity (0-99): ");
    // if input was wrong, repeat loop until correct
    getInput(minHum);
  }
  Serial.println(minHum);
  
  // enter maximum humidity
  Serial.print("Enter Maximum Humidity (");
  Serial.print(minHum + 1);
  Serial.println("-100): ");
  getInput(maxHum);
  // checks if input is in correct interval
  while (maxHum <= minHum || maxHum > 100)
  {
    Serial.print("Enter Maximum Humidity (");
    Serial.print(minHum + 1);
    Serial.println("-100): ");
    // if input was wrong, repeat loop until correct
    getInput(maxHum);
  }
  Serial.println(maxHum);
  
  // enter minimum soil moisture
  Serial.println("Enter Minimum Soil Moisture (0-99): ");
  getInput(minMoist);
  // checks if input is in correct interval
  while (minMoist < 0 || minMoist > 99)
  {
    Serial.println("Enter Minimum Soil Moisture (0-99): ");
    // if input was wrong, repeat loop until correct
    getInput(minMoist);
  }
  Serial.println(minMoist);
  
  // enter maximum soil moisture
  Serial.print("Enter Maximum Soil Moisture (");
  Serial.print(minMoist + 1);
  Serial.println("-100): ");
  getInput(maxMoist);
  // checks if input is in correct interval
  while (maxMoist <= minMoist || maxMoist > 100)
  {
    Serial.print("Enter Maximum Soil Moisture (");
    Serial.print(minMoist + 1);
    Serial.println("-100): ");
    // if input was wrong, repeat loop until correct
    getInput(maxMoist);
  }
  Serial.println(maxMoist);
  
  // store the parameters into an array
  minMaxes[0] = minTemp;
  minMaxes[1] = maxTemp;
  minMaxes[2] = minHum;
  minMaxes[3] = maxHum;
  minMaxes[4] = minMoist;
  minMaxes[5] = maxMoist;
  
  // send array of parameters to analyzer arduino
  sendAnalyze(minMaxes);
  
  // enter current hour
  Serial.println("Enter Current Hour (0-23): ");
  getInput(curHour);
  
  // checks if input is in correct interval
  while (curHour < 0 || curHour > 22)
  {
    Serial.println("Enter Current Hour (0-23): ");
    
    // if input was wrong, repeat loop until correct
    getInput(curHour);
  }
  Serial.println(curHour);
  
  // enter current minute
  Serial.println("Enter Current Minute (0-59): ");
  getInput(curMin);
  
  // checks if input is in correct interval
  while (curMin < 0 || curMin > 59)
  {
    Serial.println("Enter Current Minute (0-59): ");
    
    // if input was wrong, repeat loop until correct
    getInput(curMin);
  }
  Serial.println(curMin);
  
  // enter current second
  Serial.println("Enter Current Second (0-59): ");
  getInput(curSec);
  
  // checks if input is in correct interval
  while (curSec < 0 || curSec > 59)
  {
    Serial.println("Enter Current Second (0-59): ");
    
    // if input was wrong, repeat loop until correct
    getInput(curSec);
  }
  Serial.println(curSec);
  setTime(curHour, curMin, curSec, 0, 0, 0);
  lastRead = minute();
}
