/* MTG PILLMACHINE MASTER
 * moja & circuitcircus
 *
 * Dispenser machine
 */


// Includes
#include <SPI.h>
#include <MFRC522.h>

#include <Servo.h>


// RFID variables
#define RST_PIN 9
#define SS_PIN 10

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

String readId = "";
String lastReadId = "";

byte readCard[4];    // Stores scanned ID read from RFID Module

// Servo variables
Servo servoNFC;
Servo servoDispenser;

int posNFC;
int posDispenser;

// Button variables
int buttonPin = 2;
int buttonState = 0;
int lastButtonState = 0;


// System variables
boolean hasReadNFC = false;
boolean hasPressedButton = false;


void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  SPI.begin();
  mfrc522.PCD_Init(); // Init MFRC522 (RFID reader)

  // Attach servos
  servoNFC.attach(3);
  servoDispenser.attach(5);

  servoNFC.write(0);  

  pinMode(buttonPin, INPUT);

  // Ready to start
  Serial.println("Ready");
}

void loop() {

  buttonState = digitalRead(buttonPin);
  Serial.println(buttonState);
  
  if(!hasReadNFC) {
    do {
      readId = getID();
      if (readId.length() > 0) { // An NFC chip has been presented
        hasReadNFC = true;
        delay(1000);
        openServo(posNFC, servoNFC); // Close NFC coin slot
        delay(1000);
      }
    } while (readId.length() > 0);
  }

  // If an NFC coin has been read, we can start listening for a button press from the user
  if(hasReadNFC && !hasPressedButton) {
    if (buttonState != lastButtonState) {
      if (buttonState == HIGH) { // User has pressed button
        hasPressedButton = true;
        openServo(posDispenser, servoDispenser); // Dispense pills

        delay(1000);

        // Pills have now been dispensed and we reset the system
        closeServos();
        hasReadNFC = false;
        hasPressedButton = false;
      }
      delay(50);
    }
    lastButtonState = buttonState;
  }
}


String getID() {

  String id = "";

  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return id;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return id;
  }

  for (int i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    id += readCard[i];
  }

  Serial.println(id);

  mfrc522.PICC_HaltA(); // Stop reading
  return id;
}

void openServo(int servoPos, Servo servo) {
  for (servoPos = 0; servoPos <= 180; servoPos += 1) {
    servo.write(servoPos);  
    delay(15);
  }
  return;
}

void closeServos() {
  for (int pos = 180; pos >= 0; pos -= 1) { 
    servoNFC.write(pos);   
    servoDispenser.write(pos);   
    delay(15);
  }
  return;
}



