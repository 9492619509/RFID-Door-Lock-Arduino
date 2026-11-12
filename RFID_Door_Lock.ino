/*
  Enhanced Security RFID Door Lock System (Arduino UNO)

  Pin connections (from the circuit diagram):
    MFRC522:  SDA(SS) -> D10, SCK -> D13, MOSI -> D11, MISO -> D12,
              RST -> D9, 3.3V -> 3.3V, GND -> GND
    Green LED -> D3 (through 220 ohm resistor)
    Red LED   -> D4 (through 220 ohm resistor)
    Buzzer    -> D5
    Servo     -> D6 (signal), 5V (VCC), GND

  Libraries: MFRC522 (by GithubCommunity), SPI and Servo (built in).
*/

#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// ---------- Pin definitions ----------
#define SS_PIN     10
#define RST_PIN    9
#define GREEN_LED  3
#define RED_LED    4
#define BUZZER     5
#define SERVO_PIN  6

// ---------- Settings ----------
const int LOCKED_ANGLE   = 0;     // servo angle when door is locked
const int UNLOCKED_ANGLE = 90;    // servo angle when door is unlocked
const unsigned long UNLOCK_TIME = 3000;  // door stays open for 3 seconds

// Authorised card UID (4 bytes). Replace with your own card's UID.
// Upload the sketch, open Serial Monitor (9600 baud), scan your card
// and copy the printed UID here.
byte authorizedUID[4] = {0xDE, 0xAD, 0xBE, 0xEF};

MFRC522 rfid(SS_PIN, RST_PIN);
Servo doorServo;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  doorServo.attach(SERVO_PIN);
  doorServo.write(LOCKED_ANGLE);   // start with the door locked

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);

  Serial.println(F("RFID Door Lock System ready."));
  Serial.println(F("Place a card near the reader..."));
}

void loop() {
  // Wait for a new card
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  printUID();

  if (isAuthorized()) {
    accessGranted();
  } else {
    accessDenied();
  }

  // Stop communication with the card
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// Print the scanned card UID on the Serial Monitor
void printUID() {
  Serial.print(F("Card UID:"));
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();
}

// Compare the scanned UID with the stored authorised UID
bool isAuthorized() {
  if (rfid.uid.size != sizeof(authorizedUID)) return false;
  for (byte i = 0; i < sizeof(authorizedUID); i++) {
    if (rfid.uid.uidByte[i] != authorizedUID[i]) return false;
  }
  return true;
}

// Authorised card: green LED, short beep, unlock door, then lock again
void accessGranted() {
  Serial.println(F("Access Granted"));
  digitalWrite(GREEN_LED, HIGH);
  beep(1, 150);
  doorServo.write(UNLOCKED_ANGLE);
  delay(UNLOCK_TIME);
  doorServo.write(LOCKED_ANGLE);
  digitalWrite(GREEN_LED, LOW);
}

// Unauthorised card: red LED and buzzer alert, door stays locked
void accessDenied() {
  Serial.println(F("Access Denied"));
  digitalWrite(RED_LED, HIGH);
  beep(3, 200);
  delay(1000);
  digitalWrite(RED_LED, LOW);
}

// Sound the buzzer 'times' times, each beep lasting 'duration' ms
void beep(int times, int duration) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(duration);
    digitalWrite(BUZZER, LOW);
    delay(duration);
  }
}
