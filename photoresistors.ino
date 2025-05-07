#include <Servo.h>


// Giving all used pins a friendly name
const int photoLeft = A0;
const int photoFront = A1;
const int photoRight = A2;

const int sonarTrig = 8;
const int sonarEcho = 9;

int wheelLeftF = 11;
int wheelLeftR = 10;
int wheelRightF = 6;
int wheelRightR = 5;

int sprayServo = 13;

Servo spray;

// Initializing sensor values
int valueLeft;
int valueFront;
int valueRight;

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

int maxDistanceValue = 100; // Maximum value that photoFront can take before stopping wheels

int maxWallDistanceValue = 100; // Maximum value that sonar can take before turning around

int minimumFireValue = 100; // Minimum value that a photoresistor needs before it considers a light to be a fire

int wheelSpeed = 60; // RANGE: 0 - 255, adjusts motor values. Values above 75 might make it explode

bool elonMode = 0; // Don't mind this

// END PARAMETERS //

void turnLeft() {
    analogWrite(wheelLeftF, 0);
    analogWrite(wheelLeftR, wheelSpeed);
    analogWrite(wheelRightF, wheelSpeed);
    analogWrite(wheelRightR, 0);
}

void turnRight() {
    analogWrite(wheelLeftF, wheelSpeed);
    analogWrite(wheelLeftR, 0);
    analogWrite(wheelRightF, 0);
    analogWrite(wheelRightR, wheelSpeed);
}

void forwardDrive() {
    analogWrite(wheelLeftF, wheelSpeed);
    analogWrite(wheelLeftR, 0);
    analogWrite(wheelRightF, wheelSpeed);
    analogWrite(wheelRightR, 0);
}

void reverseDrive() {
    analogWrite(wheelLeftF, 0);
    analogWrite(wheelLeftR, wheelSpeed);
    analogWrite(wheelRightF, 0);
    analogWrite(wheelRightR, wheelSpeed);
}

void allStop() {
    analogWrite(wheelLeftF, 0);
    analogWrite(wheelLeftR, 0);
    analogWrite(wheelRightF, 0);
    analogWrite(wheelRightR, 0);
}

void allBrake() {
    analogWrite(wheelLeftF, wheelSpeed);
    analogWrite(wheelLeftR, wheelSpeed);
    analogWrite(wheelRightF, wheelSpeed);
    analogWrite(wheelRightR, wheelSpeed);
}

float getSonarDistance() {
    // Since we have a stupid Sonar sensor, we have to do the math ourselves
    // Code snippets interpreted from https://projecthub.arduino.cc/Isaac100/getting-started-with-the-hc-sr04-ultrasonic-sensor-7cabe1
    digitalWrite(sonarTrig, LOW);
    delayMicroseconds(2);
    digitalWrite(sonarTrig, HIGH);
    delayMicroseconds(10);
    digitalWrite(sonarTrig, LOW);

    durationSonar = pulseIn(sonarEcho, HIGH);

    distanceSonar = (durationSonar * 0.0343) / 2;

    return distanceSonar;
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
    pinMode(wheelRightF, OUTPUT);
    pinMode(wheelRightR, OUTPUT);
    
    pinMode(sprayServo, OUTPUT);
    
    spray.attach(sprayServo);
    spray.write(0);
    
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
}

void loop() {
    // Read and print the photoresistor values each loop iteration
    valueLeft = analogRead(photoLeft);
    valueFront = analogRead(photoFront);
    valueRight = analogRead(photoRight);
    
    Serial.println(valueLeft);
    Serial.println(valueFront);
    Serial.println(valueRight);
    
    valueSonar = getSonarDistance();

    Serial.println(valueSonar);

    // The values don't update nicely in the first run, so we skip that one
  	if (initialRun) {
      	initialRun = 0;
    	return;
  	}
  
    // Evaluate all the booleans
    maxDistanceReached = (valueFront > maxDistanceValue);
    frontLargest = (valueFront > valueLeft and valueFront > valueRight);
    sourceCentered = ((abs(valueLeft - valueRight) < errorMargin) and not (valueFront == valueLeft and valueFront == valueRight));
    fireExists = ((valueSonar < maxWallDistanceValue) and (valueFront > minimumFireValue));

    // Extinguisher, if conditions are met, it loops the main loop up until this function until the fire is gone
    if (maxDistanceReached and fireExists) {
        Serial.println("Extinguishing Fire");
      	allStop();
        spray.write(90);
        delay(1000);
        spray.write(0);
        delay(800);
        return;
    }
    // End extinguisher


    // Pathfinding code
    if (fireFound and not maxDistanceReached and not sourceCentered) {
        Serial.println("Pathfinding code running");
        while (valueLeft > valueFront) {
            turnLeft();
            delay(200);
            allStop();
            valueLeft = analogRead(photoLeft);
            valueFront = analogRead(photoFront);
        }

        while (valueRight > valueFront) {
            turnRight();
            delay(200);
            allStop();
            valueRight = analogRead(photoRight);
            valueFront = analogRead(photoFront);
        }
    }

    if (not maxDistanceReached) {
        if (sourceCentered and frontLargest) {
            forwardDrive();
            delay(200);
            allStop();
        } else if (valueLeft > valueRight) {
            turnLeft();
            delay(200);
            allStop();
        } else if (valueRight > valueLeft) {
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

        fireFound = (valueLeft > minimumFireValue or valueRight > minimumFireValue or valueFront > minimumFireValue);

        if (fireFound) {
            Serial.println("Fire found!");
            return;
        } else {
            Serial.println("No fire found, restarting loop");
        }

    }
    // End search loop
}