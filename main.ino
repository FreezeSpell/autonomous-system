// Giving all used pins a friendly name
const int photoLeft = A0;
const int photoFront = A1;
const int photoRight = A2;

const int sonarTrig = 8;
const int sonarEcho = 9;

int wheelLeftF = 2;
int wheelLeftR = 4;
int wheelLeftEN = 3;
int wheelRightF = 7;
int wheelRightR = 6;
int wheelRightEN = 5;

int alarm = 10;

int blueLed = 11;
int redLed = 12;

// Initializing sensor values
float valueLeft;
float valueFront;
float valueRight;

float durationSonar; // Value in microseconds
float distanceSonar; // Value in centimeters
float valueSonar; // Value in centimeters

// Initializing conditional booleans
bool maxDistanceReached; // Checks whether agent is too close to source
bool frontLargest; // Checks whether front sensor gives highest output
bool sourceCentered; // Checks whether source is centered between left and right sensors
bool fireExists; // Speaks for itself
bool fireFound; // Also
bool initialRun = 1; // Initial run has some goofy issues, so we skip the entire loop after setting sensor values

// PARAMETERS //

int errorMargin = 100; // Margin that is allowed for source light to be considered centered

int maxFireValue = 320; // Maximum value that photoFront can take before stopping wheels

int maxWallDistanceValue = 100; // Maximum value that sonar can take before turning around

int minimumFireValue = 160; // Minimum value that a photoresistor needs before it considers a light to be a fire

int wheelSpeed = 85; // RANGE: 0 - 255, adjusts motor values. Values above 75 might make it explode

bool elonMode = 0; // Don't mind this

// END PARAMETERS //

void turnLeft() {
  	digitalWrite(wheelRightF, HIGH);
  	digitalWrite(wheelLeftR, HIGH);
  	analogWrite(wheelLeftEN, wheelSpeed);
  	analogWrite(wheelRightEN, wheelSpeed);
    digitalWrite(redLed, HIGH);
}

void turnRight() {
    digitalWrite(wheelRightR, HIGH);
  	digitalWrite(wheelLeftF, HIGH);
  	analogWrite(wheelLeftEN, wheelSpeed);
  	analogWrite(wheelRightEN, wheelSpeed);
    digitalWrite(blueLed, HIGH);
}

void forwardDrive() {
    digitalWrite(wheelRightF, HIGH);
  	digitalWrite(wheelLeftF, HIGH);
  	analogWrite(wheelLeftEN, wheelSpeed);
  	analogWrite(wheelRightEN, wheelSpeed);
    digitalWrite(blueLed, HIGH);
    digitalWrite(redLed, HIGH);
}

void reverseDrive() {
    digitalWrite(wheelRightR, HIGH);
  	digitalWrite(wheelLeftR, HIGH);
  	analogWrite(wheelLeftEN, wheelSpeed);
  	analogWrite(wheelRightEN, wheelSpeed);
}

void allStop() {
    digitalWrite(wheelRightF, LOW);
  	digitalWrite(wheelLeftR, LOW);
    digitalWrite(wheelLeftF, LOW);
    digitalWrite(wheelRightR, LOW);
  	analogWrite(wheelLeftEN, 0);
  	analogWrite(wheelRightEN, 0);
}

void runAlarm(int loops) {
    for (int i = 0; i < loops; i++) {
        digitalWrite(blueLed, HIGH);
        analogWrite(alarm, 127);
        delay(500);
        digitalWrite(blueLed, LOW);
        digitalWrite(redLed, HIGH);
        analogWrite(alarm, 255);
        delay(500);
        digitalWrite(redLed, LOW);
    }
    digitalWrite(redLed, LOW);
    digitalWrite(blueLed, LOW);
    analogWrite(alarm, 0);
}

void setup() {
    // Pin declarations for sensors
    pinMode(photoLeft, INPUT);
    pinMode(photoFront, INPUT);
    pinMode(photoRight, INPUT);
    
    pinMode(sonarEcho, INPUT);
    pinMode(sonarTrig, OUTPUT); // Output because the Sonar module requires a trigger to function. Not technically a sensor, but... deal with it
    
    // Pin declarations for servos
    pinMode(wheelLeftF, OUTPUT);
    pinMode(wheelLeftR, OUTPUT);
    pinMode(wheelLeftEN, OUTPUT);
    pinMode(wheelRightF, OUTPUT);
    pinMode(wheelRightR, OUTPUT);
    pinMode(wheelRightEN, OUTPUT);
    
    pinMode(blueLed, OUTPUT);
    pinMode(redLed, OUTPUT);

    pinMode(alarm, OUTPUT);
    
    Serial.begin(9600); // Don't change this, it messes with the output console

    if (elonMode) {
        // Puts itself into a situation where it shouldn't be, hallucinates a problem, declares it solved the problem, then does nothing until you forcefully kill the process
        forwardDrive();
        delay(500);
        allStop();

        fireExists = 1;
        Serial.println("I extinguished the fire!");
        while (1) {
            delay(1000);
        }
    }

    // Countdown before main loop starts, so it doesn't immediately run off when uploading code
    // Yes, it's repetitive and ugly
    delay(1000);
    digitalWrite(blueLed, HIGH);
    delayMicroseconds(5000);
    digitalWrite(blueLed, LOW);
    delay(1000);
    digitalWrite(blueLed, HIGH);
    delayMicroseconds(5000);
    digitalWrite(blueLed, LOW);
    delay(1000);
    digitalWrite(blueLed, HIGH);
    analogWrite(alarm, 127);
    delayMicroseconds(5000);
    digitalWrite(blueLed, LOW);
    analogWrite(alarm, 0);
    delay(1000);
    digitalWrite(redLed, HIGH);
    digitalWrite(blueLed, HIGH);
    analogWrite(alarm, 127);
    delayMicroseconds(5000);
    digitalWrite(redLed, LOW);
    digitalWrite(blueLed, LOW);
    analogWrite(alarm, 0);
    delay(1000);
    digitalWrite(redLed, HIGH);
    digitalWrite(blueLed, HIGH);
    analogWrite(alarm, 127);
    delayMicroseconds(5000);
    digitalWrite(redLed, LOW);
    digitalWrite(blueLed, LOW);
    analogWrite(alarm, 0);
    delay(1000);
}

void loop() {
    // Read and print the photoresistor values each loop iteration
    valueLeft = analogRead(photoLeft);
    valueFront = analogRead(photoFront);
    valueRight = analogRead(photoRight);

    digitalWrite(blueLed, LOW);
    digitalWrite(redLed, LOW);
    
    Serial.println(valueLeft);
    Serial.println(valueFront);
    Serial.println(valueRight);

    // The values don't update nicely in the first run, so we skip that one
  	if (initialRun) {
      	initialRun = 0;
    	return;
  	}
  
    // Evaluate all the booleans
    maxDistanceReached = (valueFront > maxFireValue);

    frontLargest =      (valueFront > valueLeft 
                        and valueFront > valueRight);

    sourceCentered =    ((abs(valueLeft - valueRight) < errorMargin) 
                        and not (valueFront == valueLeft and valueFront == valueRight));

    fireExists = (valueFront > minimumFireValue);

    // Alarm, if conditions are met is looped
    if (maxDistanceReached and fireExists) {
        Serial.println("Alarming Fire");
      	allStop();
        runAlarm(5);
        fireFound = 0;
        fireExists = 0;
        initialRun = 1;
        return;
    }
    // End alarm


    // Pathfinding code
    if (fireFound and not maxDistanceReached and not sourceCentered) {
        Serial.println("Pathfinding code running");
        while (valueLeft > valueFront) {
            allStop();
            delayMicroseconds(2);
            turnLeft();
            delay(200);
            allStop();
            valueLeft = analogRead(photoLeft);
            valueFront = analogRead(photoFront);
        }

        while (valueRight > valueFront) {
            allStop();
            delayMicroseconds(2);
            turnRight();
            delay(200);
            allStop();
            valueRight = analogRead(photoRight);
            valueFront = analogRead(photoFront);
        }
    }

    if (not maxDistanceReached) {
        if (sourceCentered and frontLargest) {
            allStop();
            delayMicroseconds(2);
            forwardDrive();
            delay(200);
            allStop();
        } else if (valueLeft > valueRight) {
            allStop();
            delayMicroseconds(2);
            turnLeft();
            delay(200);
            allStop();
        } else if (valueRight > valueLeft) {
            allStop();
            delayMicroseconds(2);
            turnRight();
            delay(200);
            allStop();
        } 
    }
    // End pathfinding code

    // Searching loop: only gets executed if all other checks fail, therefore means only searches if no fire that satisfies the conditions is found
    while (not fireFound) {
        Serial.println("Searching loop running");
        allStop();
        turnLeft();

        delay(200);
        allStop();

        // Check new values
        valueLeft = analogRead(photoLeft);
        valueFront = analogRead(photoFront);
        valueRight = analogRead(photoRight);

        valueLeft = analogRead(photoLeft);
        valueFront = analogRead(photoFront);
        valueRight = analogRead(photoRight);

        fireFound = (valueLeft > minimumFireValue or valueRight > minimumFireValue or valueFront > minimumFireValue);

        if (fireFound) {
            Serial.println("Fire found!");
            return;
        } else {
            Serial.println("No fire found, restarting loop");
        }

    }
    // End search loop
    delay(500);
}
