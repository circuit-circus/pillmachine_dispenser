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
Servo servoLDR;

int posNFC = 0;
int posDispenser = 0;
int posLDR = 0;

// Button variables
int buttonPinDanish = 2;
int buttonPinEnglish = 3;
int buttonStateDanish = 0;
int buttonStateEnglish = 0;
int lastButtonStateDanish = 0;
int lastButtonStateEnglish = 0;

// LDR variables
int pinLDR = A0;
int valLDR;
int thresholdLDR = 150;

// System variables
boolean hasReadNFC = false;
boolean hasPressedButton = false;
boolean hasTriedToDispensePill = false;
boolean hasDispensedPill = false;


unsigned long triedToDispenseMillis = 0;


void setup() {
  Serial.begin(9600);
  while (!Serial);

  SPI.begin();
  mfrc522.PCD_Init(); // Init MFRC522 (RFID reader)

  // Attach servos
  servoNFC.attach(5);
  servoDispenser.attach(6);
  servoLDR.attach(7);

  servoNFC.write(posNFC);
  servoDispenser.write(posDispenser);
  servoLDR.write(posLDR);

  pinMode(buttonPinDanish, INPUT);
  pinMode(buttonPinEnglish, INPUT);

  // Ready to start
  Serial.println("Ready");

}

void loop() {

  // PHASE 1 : Check for a token
  if (!hasReadNFC) {
    checkForNFC();
  }

  // PHASE 2 : Check for buttonpress (English or Danish)
  if (hasReadNFC && !hasPressedButton) {
    checkForButtonPress();
  }

  // PHASE 3 : Try to dispense pill(s)
  if (hasReadNFC && hasPressedButton) {
    tryToDispensePill();
    printDiagnosis();
  }

  // PHASE 4 : Check if pill(s) were acually dispensed
  unsigned long currentMillis = millis();
  if (hasTriedToDispensePill) {
    valLDR = analogRead(pinLDR);
    if (valLDR < thresholdLDR) { // Pills are blocking the LDR = pills were dispensed
      dispenseFromPillChecker();
      
    } else if (currentMillis - triedToDispenseMillis >= 2000) { // Allow 2 seconds for pills to be dispensed
      // Pills were not dispensed
      tryToDispensePill();
    }
  }
}

// CHECKS FOR A PRESENTED NFC TAG (TOKEN)
void checkForNFC() {
  do {
    readId = getID();
    if (readId.length() > 0) { // An NFC chip has been presented
      hasReadNFC = true;
      delay(1000);
      openServo(posNFC, servoNFC); // Close NFC coin slot
    }
  } while (readId.length() > 0);
}

// CHECKS IF A BUTTON HAS BEEN PRESSED
void checkForButtonPress() {
  if ( (buttonStateDanish != lastButtonStateDanish) || (buttonStateEnglish != lastButtonStateEnglish) ) {
    if ( buttonStateDanish == HIGH || buttonStateEnglish == HIGH) {
      hasPressedButton = true;
    }
    delay(50);
  }
}

// DISPENSES A PILL
void tryToDispensePill() {
  // VICTORS CODE TO DISPENSE THE PILL(S) HERE (both opens and closes the servo)
  hasTriedToDispensePill = true;
  triedToDispenseMillis = millis();
}

// PRINT DIAGOSIS
void printDiagnosis() {
  // CODE TO PRINT THE DIAGNOSIS HERE
}

// SEND PILLS TO THE USER AND FINISH UP
void dispenseFromPillChecker() {
  hasDispensedPill = true;
  openServo(posLDR, servoLDR);
  resetSystem();
}

// RESET THE SYSTEM
void resetSystem() {
  hasReadNFC = false;
  hasPressedButton = false;
  hasTriedToDispensePill = false;
  hasDispensedPill = false;
  closeServo(posLDR, servoLDR);
  closeServo(posNFC, servoNFC);

}

void openServo(int servoPos, Servo servo) {
  for (servoPos = 0; servoPos <= 180; servoPos += 1) {
    servo.write(servoPos);
    delay(15);
  }
  return;
}

void closeServo(int servoPos, Servo servo) {
  for (int pos = 180; pos >= 0; pos -= 1) {
    servo.write(servoPos);
    delay(15);
  }
  return;
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
