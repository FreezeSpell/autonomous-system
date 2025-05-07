#include <Servo.h>


// Giving all used pins a friendly name
const int photoLeft = A0;
const int photoFront = A1;
const int photoRight = A2;

const int sonarTrig = 8;
const int sonarEcho = 9;

int wheelLeftF = 7;
int wheelLeftR = 6;
int wheelRightF = 5;
int wheelRightR = 4;

int sprayServo = 11;

Servo spray;

// Initializing sensor values
int valueLeft;
int valueFront;
int valueRight;

float durationSonar;
float distanceSonar;
float valueSonar;

// Initializing conditional booleans
bool maxDistanceReached; // Checks whether agent is too close to source
bool frontLargest; // Checks whether front sensor gives highest output
bool sourceCentered; // Checks whether source is centered between left and right sensors
bool fireExists; // Speaks for itself
bool fireFound; // Also

// PARAMETERS //

int errorMargin = 40; // Margin that is allowed for source light to be considered centered

int maxDistanceValue = 100; // Maximum value that photoFront can take before stopping wheels

int maxWallDistanceValue = 100; // Maximum value that sonar can take before turning around

int minimumFireValue = 50; // Minimum value that a photoresistor needs before it considers a light to be a fire

// END PARAMETERS //

void turnLeft() {
    digitalWrite(wheelLeftF, LOW);
    digitalWrite(wheelLeftR, HIGH);
    digitalWrite(wheelRightR, LOW);
    digitalWrite(wheelRightF, HIGH);
}

void turnRight() {
    digitalWrite(wheelLeftF, HIGH);
    digitalWrite(wheelLeftR, LOW);
    digitalWrite(wheelRightR, HIGH);
    digitalWrite(wheelRightF, LOW);
}

void forwardDrive() {
    digitalWrite(wheelLeftF, HIGH);
    digitalWrite(wheelRightF, HIGH);
    digitalWrite(wheelLeftR, LOW);
    digitalWrite(wheelRightR, LOW);
}

void reverseDrive() {
    digitalWrite(wheelLeftF, LOW);
    digitalWrite(wheelRightF, LOW);
    digitalWrite(wheelLeftR, HIGH);
    digitalWrite(wheelRightR, HIGH);
}

void allStop() {
    digitalWrite(wheelLeftF, LOW);
    digitalWrite(wheelRightF, LOW);
    digitalWrite(wheelLeftR, LOW);
    digitalWrite(wheelRightR, LOW);
}

void allBrake() {
    digitalWrite(wheelLeftF, HIGH);
    digitalWrite(wheelRightF, HIGH);
    digitalWrite(wheelLeftR, HIGH);
    digitalWrite(wheelRightR, HIGH);
}

float getSonarDistance() {
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
    pinMode(sonarTrig, OUTPUT);
    
    // Pin declarations for servos
    pinMode(wheelLeftF, OUTPUT);
    pinMode(wheelLeftR, OUTPUT);
    pinMode(wheelRightF, OUTPUT);
    pinMode(wheelRightR, OUTPUT);
    
    pinMode(sprayServo, OUTPUT);
    
    spray.attach(sprayServo);
    
    Serial.begin(9600); // Don't change this, it messes with the output console
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

    maxDistanceReached = (valueFront < maxDistanceValue);
    frontLargest = (valueFront > valueLeft and valueFront > valueRight);
    sourceCentered = (abs(valueLeft - valueRight) < errorMargin);
    fireExists = (valueSonar > maxWallDistanceValue and valueFront > minimumFireValue);

    // Extinguisher, if fire exists will loop
    if (maxDistanceReached and fireExists) {
        Serial.println("Extinguishing Fire");
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
        Serial.println("Searching loop running")
        allStop();

        turnLeft();

        delay(200);

        fireFound = (valueLeft > minimumFireValue or valueRight > minimumFireValue or valueFront > minimumFireValue);

        if (fireFound) {
            return;
        }
    }
    // End search loop
}