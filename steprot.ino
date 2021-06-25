//Arduino program to control the 3D scanner System

//Define the Pins and declare direction variable
const int dirPin = 2;
const int stpPin = 3;
const int slpPin = 4;
const int homePin = 5;
const int shutterGnd = 10;
const int shutter = 9;
bool dir;

// choose amount of steps per given number:
// moveOneStp
// moveFullStp
// move64Stps // 100stpProtation
#define moveStepper move64Stps

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

//Main Programm
void loop() {
  byte serData[3] = {0,0,0};  //array of 3 bytes, serial data will be soterd here
  int numStps = 0;  // number of steps to take
  if (Serial.available() > 0) {
    int count = 0; // limits recieved bytes to 3
    while (Serial.available() > 0 && count < 3) {
      serData[count] = Serial.read();
      count++;
      delay(10); // important, serial port if slower than adruino can read data  
    }
    if (Serial.available() > 0) { // all bytes after the first 3 are dumped, program will continue
      Serial.println("Error: too many bytes!, all but first 3 bytes ignored!");
      while (Serial.available() > 0) {
        byte tmp = Serial.read();
      }
    }
    if (serData[0] == 'H') { // if first byte is H, start homing, 2nd and 3rd byte is ignored
      homingCycle();
      serData[0] = 0;
      serData[1] = 0;
      serData[2] = 0;
    } else if (serData[0] == 'P') { // if  first byte is P, take a photo 2nd and 3rd byte is ignored
      takePic();
      serData[0] = 0;
      serData[1] = 0;
      serData[2] = 0;
    } else if ((serData[0] == 'L' || serData[0] == 'R') && ((serData[1] <= '9' && serData[1] >= '0') || serData[1] == 0)  && ((serData[2] <= '9' && serData[2] >= '0') || serData[2] == 0)) {
      // checking if input is correct
      if (serData[0] == 'L') { // set desired direction
        digitalWrite(dirPin,LOW);
      } else {
        digitalWrite(dirPin,HIGH);
      }
      digitalWrite(slpPin,LOW); // wake driver up
      delayMicroseconds(200);
      // calculate number of steps from two bytes
      if (serData[2] == 0) {
        numStps = serData[1] - 48;
      } else {
        numStps = (serData[1] - 48) * 10 + (serData[2] - 48);
      }
      for (int i = 0; i < numStps; i++) { // move desired amount of steps
        moveStepper();
        delay(1);
      }
      digitalWrite(slpPin,HIGH); // put driver to sleep
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

// method for 100 setps for a full rotation
void move64Stps() {
  for (int i = 0; i < 64; i++) {
    moveOneStp();
  }
}

// method to move a full step
void moveFullStp() {
  for (int i = 0; i < 16; i++) {
    moveOneStp();
  }
}


// method to home the machine
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
