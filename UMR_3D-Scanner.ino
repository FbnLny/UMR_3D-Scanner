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
// moveOneStp - one microstep
// moveFullStp - 16 microsteps
// move64Stps - 100 stps / rotation
#define moveStepper move64Stps

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
  digitalWrite(slpPin,HIGH);
  digitalWrite(dirPin, LOW);
}

//Main program
void loop() {
  byte serData[3] = {0,0,0};  //array of 3 bytes, serial data will be soterd here
  int numStps = 0;  // number of steps to take
  if (Serial.available() > 0) {
    int count = 0; // limits recieved bytes to 3
    while (Serial.available() > 0 && count < 3) {
      serData[count] = Serial.read(); // reads 3 bytes from buffer
      count++;
      delay(10); // important, serial port is slower than arduino can read data  
    }
    if (Serial.available() > 0) { // all bytes after the first 3 are dumped, program will continue
      Serial.println("Error: too many bytes!, all but first 3 bytes ignored!");
      while (Serial.available() > 0) {
        byte tmp = Serial.read();
      }
    }
    if (serData[0] == 'H') { // if first byte is H, start homing, 2nd and 3rd byte is ignored
      homingCycle();
    } else if (serData[0] == 'P') { // if  first byte is P, take a photo 2nd and 3rd byte is ignored
      takePic();
    } else if ((serData[0] == 'L' || serData[0] == 'R') && ((serData[1] <= '9' && serData[1] >= '0') || serData[1] == 0)  && ((serData[2] <= '9' && serData[2] >= '0') || serData[2] == 0)) {
      // checking if input is correct
      if (serData[0] == 'L') { // set desired direction
        digitalWrite(dirPin,LOW);
      } else {
        digitalWrite(dirPin,HIGH);
      }
      digitalWrite(slpPin,LOW); // wake driver up
      delayMicroseconds(200);
      // calculate number of steps from 2nd or 2nd and 3rd byte
      if (serData[2] == 0) {
        numStps = serData[1] - 48;
      } else {
        numStps = (serData[1] - 48) * 10 + (serData[2] - 48);
      }
      for (int i = 0; i < numStps; i++) { // move desired number of steps
        moveStepper();
        delay(1);
      }
      digitalWrite(slpPin,HIGH); // put driver back to sleep
      Serial.println("Moved ");
      while (Serial.available() > 0) { // dump all remaining bytes from the serial buffer
        byte tmp = Serial.read();
      }
    } else {
      Serial.println("Error: incorrect command!");
    }
  }
  serData[0] = 0; // clear serial data variables
  serData[1] = 0;
  serData[2] = 0;
}

// method to move on microstep, driver is hardwired for 16 microsteps
void moveOneStp() {
  digitalWrite(stpPin, HIGH);
  delayMicroseconds(200);
  digitalWrite(stpPin, LOW);
  delay(1);
}

// method to move a full step
void moveFullStp() {
  for (int i = 0; i < 16; i++) {
    moveOneStp();
  }
}

// method for 100 setps for a full rotation
void move64Stps() {
  for (int i = 0; i < 64; i++) {
    moveOneStp();
  }
}

// method to home the machine, drives in one direction until homePin is pulled low
void homingCycle() {
  int cnt = 0;
  digitalWrite(dirPin,HIGH);
  delay(1);
  digitalWrite(slpPin,LOW);
  while (digitalRead(homePin) == HIGH) {
    moveFullStp();
    cnt++;
    if (cnt > 1200) {
      Serial.println("Error: no endstop reached");
      break;
    }
  }
  if (digitalRead(homePin) == LOW) {
    Serial.println("Homing done.");
  }
  digitalWrite(slpPin,HIGH);
}

//method to take a picture
void takePic() {
  digitalWrite(9, LOW);
  delay(500);
  digitalWrite(9, HIGH);
  Serial.println("Picture taken.");
}
