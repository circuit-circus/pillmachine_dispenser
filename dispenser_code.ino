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
String languageChosen = "";

void setup() {
  Serial.begin(9600);
  while (!Serial);

  SPI.begin();
  mfrc522.PCD_Init(); // Init MFRC522 (RFID reader)

  Ethernet.begin(mac, myip);
  delay(5000); // wait for ethernetcard
  aktivateEthernetSPI(true);

  // Attach servos
  servoNFC.attach(5);
  servoDispenser.attach(6);

  servoNFC.write(posNFC);
  servoDispenser.write(posDispenser);

  pinMode(buttonPinDanish, INPUT);
  pinMode(buttonPinEnglish, INPUT);

  client.connect(pc_server,80);

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

  if(cardPresent) {
    if ( mfrc522.PICC_ReadCardSerial()) {
      getID();
    }
    hasReadNFC = true;
    delay(1000);
    openServo(posNFC, servoNFC); // Close NFC coin slot
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
  if ( (buttonStateDanish != lastButtonStateDanish) || (buttonStateEnglish != lastButtonStateEnglish) ) {
    if ( buttonStateDanish == HIGH || buttonStateEnglish == HIGH) {
      hasPressedButton = true;
      languageChosen = buttonStateDanish == HIGH ? "DK" : "UK";
    }
    delay(50);
  }
}

// DISPENSES A PILL
void dispensePill() {
  // VICTORS CODE TO DISPENSE THE PILL(S) HERE (both opens and closes the servo)
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
  closeServo(posNFC, servoNFC);
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

