/* MTG PILLMACHINE MASTER
 * moja & circuitcircus
 *
 * Dispenser machine
 */


// Includes
#include <SPI.h>
#include <MFRC522.h>
#include <Ethernet.h>
#include <Servo.h>

// Ethernet variables
static uint8_t mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };
static uint8_t myip[] = {  10, 0, 0, 100 };
IPAddress pc_server(10,0,0,31);  // serverens adress

EthernetClient client;


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

// Button variables
int buttonPinDanish = 2;
int buttonPinEnglish = 3;
int buttonStateDanish = 0;
int buttonStateEnglish = 0;
int lastButtonStateDanish = 0;
int lastButtonStateEnglish = 0;

// System variables
boolean hasReadNFC = false;
boolean hasPressedButton = false;
boolean hasDispensedPill = false;


void setup() {
  Serial.begin(9600);
  while (!Serial);

  SPI.begin();
  mfrc522.PCD_Init(); // Init MFRC522 (RFID reader)

  Ethernet.begin(mac, myip);
  delay(5000); // wait for ethernetcard
  aktivateEthernetSPI(false);

  // Attach servos
  servoNFC.attach(5);
  servoDispenser.attach(6);

  servoNFC.write(posNFC);
  servoDispenser.write(posDispenser);

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
    dispensePill();
    printDiagnosis();
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
void dispensePill() {
  // VICTORS CODE TO DISPENSE THE PILL(S) HERE (both opens and closes the servo)
  hasDispensedPill = true;
  resetSystem();
}

// PRINT DIAGOSIS
void printDiagnosis() {
  // CODE TO PRINT THE DIAGNOSIS HERE
  
}

// RESET THE SYSTEM
void resetSystem() {
  hasReadNFC = false;
  hasPressedButton = false;
  hasDispensedPill = false;
  closeServo(posNFC, servoNFC);
}

void aktivateEthernetSPI(boolean x) {
  // mfrc522.PICC_HaltA();
  // skift SPI/Slave... turn RFID shield off, ethernet on (LOW=on)
  // http://tronixstuff.com/2011/05/13/tutorial-arduino-and-the-spi-bus/

  digitalWrite(SS_PIN,x);
  digitalWrite(10,!x);
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
