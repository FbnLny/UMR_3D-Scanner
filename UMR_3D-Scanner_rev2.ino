// Arduino program to control the 3D scanner System
// it expects 1, 2 or 3 bytes from the serial port. First bytes should by either 'H','P','R','L'
// 'H' homes the machine, all other bytes are ignored
// 'P' takes a photo, all other bytes are ignored
// 'R'(ight) / 'L'(eft) is the command to move the machine in the given direction, 
// it expects one or two bytes after that command, these bytes have to be ascii characters representing the numbers 0-9
// therefore in this version a maximum of 99 steps can be requested

//Define the Pins
#define dirPin 2
#define stpPin 3
#define slpPin 4
#define homePin 5
#define shutter 9
#define shutterGnd 10

// choose amount of steps per given number:
// moveFullStp
// moveOneStp
#define moveStepper moveFullStp

#define startSlp LOW

//declare direction variable
bool dir;


//initialize all pins and give them default values
void setup() {
  Serial.begin(9600);
  pinMode(dirPin, OUTPUT);  // HIGH / LOW determines the step direction
  pinMode(stpPin, OUTPUT);  // HIGH / LOW cycle performs one (micro)step 
  pinMode(slpPin, OUTPUT);  // deactivates the driver when not needed
  pinMode(homePin, INPUT_PULLUP); // detects when the homeswitch is activated
  pinMode(shutterGnd, OUTPUT);  // Pin acts as GND for the shutter
  pinMode(shutter, OUTPUT); // When Pin is pulled low, shutter is triggered
  digitalWrite(shutterGnd, LOW);
  digitalWrite(shutter, HIGH);
  digitalWrite(stpPin, LOW);
  digitalWrite(slpPin,startSlp);
  digitalWrite(dirPin, LOW);
}

//Main program
void loop() {
  byte serData[3] = {0,0,0};  //array of 3 bytes, serial data will be soterd here
  int numStps = 0;  // number of steps to take
  bool fault = false;
  if (Serial.available() > 0) {
    byte serData = Serial.read();
    Serial.println(serData);
    if (serData == 'H') {
      Serial.flush();
      homingCycle();
    } else if (serData == 'P') {
      takePic();
      Serial.flush();
    } else if (serData == 'S') {
      //digitalWrite(slpPin, HIGH);
      Serial.flush();
    } else if (serData == 'W') {
      //digitalWrite(slpPin, LOW);
      Serial.flush();
    } else if (serData == 'R' || serData == 'L') {
      if (serData == 'R') {
        digitalWrite(dirPin, HIGH);
      } else {
        digitalWrite(dirPin, LOW);
      }
      delay(50);
      while (Serial.available() > 0) {
        serData = Serial.read();
        if (!(serData <= '9' && serData >= '0')) {
          fault = true;
          Serial.println("Error: illigal character!");
        } else {
          numStps *= 10;
          numStps += (serData - 48);
        }
        delay(10);
      }
      if (!fault) {
        for (int i = 0; i < numStps; i++) { // move desired number of steps
          moveStepper();
          delay(5);
        }
        Serial.print("Moved ");
        Serial.println(numStps);
      }
      Serial.flush();
    } else {
      Serial.println("Error: incorrect command!");
      Serial.flush();
    }
  }
}

// method to move on microstep, driver is hardwired for 16 microsteps
void moveOneStp() {
  digitalWrite(stpPin, HIGH);
  delayMicroseconds(200);
  digitalWrite(stpPin, LOW);
  delay(4);
}

// method for 100 setps for a full rotation
void moveFullStp() {
  for (int i = 0; i < 16; i++) {
    moveOneStp();
  }
}

// method to home the machine, drives in one direction until homePin is pulled low
void homingCycle() {
  Serial.println("homing");
  int cnt = 0;
  digitalWrite(dirPin,HIGH);
  delay(1);
  while (digitalRead(homePin) == HIGH) {
    moveFullStp();
    cnt++;
    if (cnt > 1200) {
      Serial.println("Error: no endstop reached");
      break;
    }
    delay(10);
  }
  if (digitalRead(homePin) == LOW) {
    Serial.println("Homing done.");
  }
}

//method to take a picture
void takePic() {
  digitalWrite(9, LOW);
  delay(500);
  digitalWrite(9, HIGH);
  Serial.println("Picture taken.");
}
