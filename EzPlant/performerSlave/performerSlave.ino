Performer Slave
/* performer Arduino
  This Arduino controls a 5v relay that controls power flow to
  an outlet. The Arduino receives commands (turn on or turn off)
  from the master Arduino and performs operations based on those
  commands. The Arduino also has a button that acts as an on/off
  toggle for the 5v relay. When the button is pressed, the signal
  written to the relay will be LOW, no matter what command is
  received from the master.
*/

#include <Wire.h>

// initialize pins and variables
const int light = 4;
const int buttonPin = 7;
int buttonState;
char lightCom = ' ';
int lightState;

// for debounce check
int checkOne = 0;
int checkTwo = 0; /

/ light enable variable
bool isEnabled = true;

void setup() {
  Wire.begin(2);  // start address as 2 (performer address)
  Wire.onReceive(receiveEvent);  // when something is received from the master, call receiveEvent
  Serial.begin(9600);
  pinMode(light, OUTPUT); // initialize the digital pin as an output.
  pinMode(buttonPin, INPUT); // initialize the digital pin as an input.
}

void loop() {
  // debounce check for the button, prevents bounce and only counts clicks, not holds
  checkOne = digitalRead(buttonPin);
  delay(10);
  checkTwo = digitalRead(buttonPin);
  if (checkOne == checkTwo) {
    if (checkOne != buttonState) {
      if (checkOne == HIGH) {
        if (isEnabled == false) {
          isEnabled = true;
          Serial.println("Enable light");
          
        } else {
          isEnabled = false;
          digitalWrite(light, LOW);
          Serial.println("Disable light");
        }
      }
    }
  }
  buttonState = checkOne;
  
  lightState = digitalRead(light);
  
  // if the command received is to turn on the light and the light is enabled, turn on the light if it is not already on
  if (isEnabled == true && lightCom == 'O') {
    if (lightState != HIGH) {
      digitalWrite(light, HIGH); // turn light on
    }
  }

  // if the command received is to turn off the light and the light is enabled, turn off the light if it is not already off
  else if (isEnabled == true && lightCom == 'F') {
    if (lightState != LOW) {
      digitalWrite(light, LOW); // turn light off
    }
  }
}

// function that executes whenever data is received from master// this function is registered as an event, see setup()
void receiveEvent(int howMany) {

  // read all bytes on the Wire buffer
  while (Wire.available())  {
    lightCom = Wire.read(); // receive byte as a character
    Serial.print(lightCom); // print the character (debugging)
  }
}
