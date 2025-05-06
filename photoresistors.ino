#include <Servo.h>


// Giving all used pins a friendly name
const int photoLeft = A0;
const int photoFront = A1;
const int photoRight = A2;

const int sonar = A3;

int wheelLeft = 9;
int wheelRight = 10;

int sprayServo = 11;

Servo spray;

// Initializing sensor values
int valueLeft;
int valueFront;
int valueRight;

int valueSonar;

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

void setup() {
    // Pin declarations for sensors
    pinMode(photoLeft, INPUT);
    pinMode(photoFront, INPUT);
    pinMode(photoRight, INPUT);
    
    pinMode(sonar, INPUT);
    
    // Pin declarations for servos
    pinMode(wheelLeft, OUTPUT);
    pinMode(wheelRight, OUTPUT);
    
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
    
    valueSonar = analogRead(sonar);

    maxDistanceReached = (valueFront < maxDistanceValue);
    frontLargest = (valueFront > valueLeft and valueFront > valueRight);
    sourceCentered = (abs(valueLeft - valueRight) < errorMargin);
    fireExists = (valueSonar > maxWallDistanceValue and valueFront > minimumFireValue);

    // Extinguisher, if fire exists will loop
    if (maxDistanceReached and fireExists) {
        spray.write(90);
        delay(1000);
        spray.write(0);
        delay(800);
        return;
    }
    // End extinguisher


    // Pathfinding code
    if (fireFound and not maxDistanceReached and not sourceCentered) {
        while (valueLeft > valueFront) {
            digitalWrite(wheelRight, HIGH);
            delay(200);
            digitalWrite(wheelRight, LOW);
            valueLeft = analogRead(photoLeft);
            valueFront = analogRead(photoFront);
        }

        while (valueRight > valueFront) {
            digitalWrite(wheelLeft, HIGH);
            delay(200);
            digitalWrite(wheelLeft, LOW);
            valueRight = analogRead(photoRight);
            valueFront = analogRead(photoFront);
        }
    }

    if (not maxDistanceReached) {
        if (sourceCentered and frontLargest) {

        } else if (valueLeft > valueRight) {
            digitalWrite(wheelRight, HIGH);
            digitalWrite(wheelLeft, LOW);
            delay(200);
            digitalWrite(wheelRight, LOW);
        } else if (valueRight > valueLeft) {
            digitalWrite(wheelRight, LOW);
            digitalWrite(wheelLeft, HIGH);
            delay(200);
            digitalWrite(wheelLeft, LOW);
        } 
    }
    // End pathfinding code

    // Searching loop: only gets executed if all other checks fail, therefore means only searches if no fire that satisfies the conditions is found
    while (not fireFound) {
        digitalWrite(wheelLeft, LOW);
        digitalWrite(wheelRight, LOW);

        for (int i = 0; i < 10; i++) {
            digitalWrite(wheelRight, HIGH);
            delay(200);
            digitalWrite(wheelRight, LOW);
            fireFound = (valueLeft > minimumFireValue or valueRight > minimumFireValue or valueFront > minimumFireValue);

            if (fireFound) {
                break;
            }
        }

        if (fireFound) {
            return;
        }

        for (int i = 0; i < 10; i++) {
            digitalWrite(wheelLeft, HIGH);
            delay(200);
            digitalWrite(wheelLeft, LOW);
            fireFound = (valueLeft > minimumFireValue or valueRight > minimumFireValue or valueFront > minimumFireValue);

            if (fireFound) {
                break;
            }
        }

        if (fireFound) {
            return;
        }
    }
    // End search loop
}