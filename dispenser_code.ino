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
static uint8_t mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x01 };
static uint8_t myip[] = {  10, 0, 0, 200 };
IPAddress pc_server(10,0,0,31);  // serverens adress

EthernetClient client;


// RFID variables
#define RST_PIN 9
#define SS_PIN 8

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

boolean cardPresent = false; // DEBUG: Set this and isDebugging to true to test UI
boolean isDebugging = false; // DEBUG: Set this and cardPresent to true to test UI
boolean cardPresent_old = false;
String cardID = ""; // NB skal muligvis laves til char-array for at spare memory
String cardID_old = "";

// Servo variables
Servo servoNFC;
Servo servoDispenser;

#define servoNFCPin A2
#define servoDispenserPin A4

int posNFC = 0;
int posDispenser = 160;

// Button variables
const int buttonPinDanish = A0;
const int buttonPinEnglish = A1;
const int danishLEDPin = 3;
const int englishLEDPin = 2;
int buttonStateDanish = 0;
int buttonStateEnglish = 0;
int lastButtonStateDanish = 1;
int lastButtonStateEnglish = 1;

// Indicator LEDs
const int pillDropLedPin = 4;
const int numOneLedPin = 5;
const int numTwoLedPin = 6;
const int numThreeLedPin = 7;

// Timers
unsigned long NFCTimer = 0;
const unsigned long NFCTimerDuration = 5000;
unsigned long panicTimer = 0;
const unsigned long panicTimerDuration = 15000;
unsigned long previousPanicBlink = 0;
boolean panicBlinkState = 0;
const int panicBlinkDuration = 500; // How long should one blink last?

boolean isNFCTimerExpired = false;

// System variables
boolean hasReadNFC = false;
boolean hasPressedButton = false;
boolean hasDispensedPill = false;
String languageChosen = "";

void setup() {
  Serial.begin(9600);

  SPI.begin();
  mfrc522.PCD_Init(); // Init MFRC522 (RFID reader)

  Ethernet.begin(mac, myip);

  delay(5000); // wait for ethernetcard
  aktivateEthernetSPI(false);

  // Attach servos
  servoNFC.attach(servoNFCPin);
  servoDispenser.attach(servoDispenserPin);

  servoNFC.write(posNFC);
  servoDispenser.write(posDispenser);

  pinMode(danishLEDPin, OUTPUT);
  pinMode(englishLEDPin, OUTPUT);
  pinMode(numOneLedPin, OUTPUT);
  pinMode(numTwoLedPin, OUTPUT);
  pinMode(numThreeLedPin, OUTPUT);
  pinMode(pillDropLedPin, OUTPUT);

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
    updateTimer();
  }

  // PHASE 3 : Try to dispense pill(s)
  if (hasReadNFC && hasPressedButton) {
    dispensePill();
    printDiagnosis();
  }

  /*if(shouldMoveServo) {
    if((millis() - lastServoUpdate) > updateInterval) {
      lastServoUpdate = millis();

      if()
    }
  }*/

}

// CHECKS FOR A PRESENTED NFC TAG (TOKEN)
void checkForNFC() {

  if(cardPresent) {
    if ( mfrc522.PICC_ReadCardSerial()) {
      getID();
    }
    hasReadNFC = true;
    delay(500);

    openServo(posNFC, servoNFC); // Close NFC coin slot
    NFCTimer = millis();
    digitalWrite(numOneLedPin, LOW);
    digitalWrite(numTwoLedPin, HIGH);
  }

  if ( mfrc522.PICC_ReadCardSerial()) {
    getID();
  }

  
  cardPresent_old = cardPresent;


  // -----------------ALT HERUNDER SKAL STÅ SIDST I MAIN LOOP!

  if ( ! mfrc522.PICC_IsNewCardPresent() && !isDebugging) {
    cardPresent = false;
    return;
    delay(10);
  }

  cardPresent = true;

  // mfrc522.PICC_HaltA();
  delay(10);
}

// CHECKS IF A BUTTON HAS BEEN PRESSED
void checkForButtonPress() {
  buttonStateDanish = digitalRead(buttonPinDanish);
  buttonStateEnglish = digitalRead(buttonPinEnglish);

  if ( (buttonStateDanish != lastButtonStateDanish) || (buttonStateEnglish != lastButtonStateEnglish) ) {
    if ( buttonStateDanish == HIGH || buttonStateEnglish == HIGH) {
      hasPressedButton = true;
      languageChosen = buttonStateDanish == HIGH ? "UK" : "DK";

      digitalWrite(numTwoLedPin, LOW);
      digitalWrite(numThreeLedPin, HIGH);

      digitalWrite(danishLEDPin, buttonStateDanish);
      digitalWrite(englishLEDPin, buttonStateEnglish);
    }
    delay(50);
  }

  lastButtonStateDanish = buttonStateDanish;
  lastButtonStateEnglish = buttonStateEnglish;
}

// UPDATES TIMERS AND REACTS ON THEM
void updateTimer() {
  unsigned long currentMillis = millis();

  if(currentMillis > NFCTimer + NFCTimerDuration && !isNFCTimerExpired) {
    panicTimer = currentMillis;
    isNFCTimerExpired = true;
  }

  if(isNFCTimerExpired) {
    if(currentMillis - previousPanicBlink >= panicBlinkDuration) {
      previousPanicBlink = currentMillis;
      panicBlinkState = !panicBlinkState;
      digitalWrite(numTwoLedPin, panicBlinkState);
    }

    if(currentMillis > panicTimer + panicTimerDuration) {
      resetSystem();
      digitalWrite(numTwoLedPin, LOW);
      isNFCTimerExpired = false;
    }
  }
}

// DISPENSES A PILL
void dispensePill() {
  servoDispenser.write(160);
  delay(2000);
  servoDispenser.write(180);
  delay(500);
  servoDispenser.write(30);
  
  digitalWrite(pillDropLedPin, HIGH);

  delay(2000);
  servoDispenser.write(160);

  hasDispensedPill = true;
}

// PRINT DIAGOSIS
void printDiagnosis() {
  // CODE TO PRINT THE DIAGNOSIS HERE
  Serial.println("Printing");
  submitData();

}

// RESET THE SYSTEM
void resetSystem() {
  hasReadNFC = false;
  hasPressedButton = false;
  hasDispensedPill = false;
  languageChosen = "";
  isNFCTimerExpired = false;

  closeServo(posNFC, servoNFC);
  // closeServo(posDispenser, servoDispenser);

  digitalWrite(danishLEDPin, LOW);
  digitalWrite(englishLEDPin, LOW);
  digitalWrite(numOneLedPin, HIGH);
  digitalWrite(numTwoLedPin, LOW);
  digitalWrite(numThreeLedPin, LOW);
  digitalWrite(pillDropLedPin, LOW);
}

void submitData() {

  aktivateEthernetSPI(true);

  String datastring = "GET /machine//readval.php?tag=" + String(cardID) + "&lingo=" + languageChosen + " HTTP/1.0";

  if(client.connect(pc_server,80)) {
      client.println(datastring);
      client.println("Connection: close");
      client.println(); //vigtigt at sende tom linie
      client.stop();
      delay(100);
    }
  

  aktivateEthernetSPI(false);
  resetSystem();
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
    delay(10);
  }
  return;
}

void closeServo(int servoPos, Servo servo) {
  for (servoPos = 180; servoPos >= 0; servoPos -= 1) {
    servo.write(servoPos);
    delay(10);
  }
  return;
}


void getID() {
  String  cardIDtmp = "";
  // for (byte i = 0; i < mfrc522.uid.size; i++)
  for (byte i = 0; i < 3; i++) {
    byte tmp = (mfrc522.uid.uidByte[i]);
    cardIDtmp.concat(tmp);
  }

  // has ID changed?
  if (cardIDtmp != cardID_old) {
    cardID = cardIDtmp;
    cardID_old = cardID;
  }
}

