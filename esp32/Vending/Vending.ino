/*
 * 
 * ██╗░░░░░██╗██████╗░██████╗░░█████╗░██╗░░░░░██╗███████╗███╗░░██╗
 * ██║░░░░░██║██╔══██╗██╔══██╗██╔══██╗██║░░░░░██║██╔════╝████╗░██║
 * ██║░░░░░██║██████╦╝██████╔╝███████║██║░░░░░██║█████╗░░██╔██╗██║
 * ██║░░░░░██║██╔══██╗██╔══██╗██╔══██║██║░░░░░██║██╔══╝░░██║╚████║
 * ███████╗██║██████╦╝██║░░██║██║░░██║███████╗██║███████╗██║░╚███║
 * ╚══════╝╚═╝╚═════╝░╚═╝░░╚═╝╚═╝░░╚═╝╚══════╝╚═╝╚══════╝╚═╝░░╚══╝
 *
 * Libralien Vending Machine
 *
 * RFID-BC522
 * WIRING     ESP32
 * Rst        G4
 * IRQ        SKIP
 * MISO       G19
 * MOSI       G23
 * SCK        G8
 * SDA        G5

 * I2C LCD
 * SDA        G21
 * SCL        G22

*/

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Keypad.h>
#include <ESP32Servo.h>

#include "config.h"
#include "Item.h"
#include "keypad_settings.h"
#include "servo_settings.h"

#define RST_PIN 4  // Card Reader SPI settings
#define SS_PIN 5   // Card Reader SPI settings

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
String host = API_ENDPOINT;

String vending_id = VENDING_ID;
String api_key = SECRET_KEY;

LiquidCrystal_I2C lcd(0x27, 16, 2);

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

Servo servo1; 
Servo servo2; 
Servo servo3; 
Servo servo4;

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);

Item items[4];

WiFiClient client;
//*****************************************************************************************//
void setup() {
  Serial.begin(115200);       // Initialize serial communications with the PC
  SPI.begin();                // Init SPI bus
  mfrc522.PCD_Init();         // Init MFRC522 card

  // Attach all servo motors
  servo1.attach(pinServo1);
  servo2.attach(pinServo2);
  servo3.attach(pinServo3);
  servo4.attach(pinServo4);
  
  // LCD
  lcd.init();                 // Initialize the LCD
  lcd.backlight();            // Turn on the LCD backlight

  WiFi.begin(ssid, password); // Begin connection to the wifi
  Serial.println("Connecting...");
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print("Wifi");


  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  // Sync data to the server
  while (1) {
    if (syncdata(client)) {
      break;
    }
  }

  // Free resources
  // http.end();
}
//*****************************************************************************************//

int state = 0;
char selected_char;
int selected;
unsigned long start_time = 0;

//*****************************************************************************************//
void loop() {
  if (state == 0) {
    // INITIALIZATION STATE

    // Set the selected char to char \0
    selected_char = '\0';
    // Set the selected item to 0
    selected = 0;
    // Write Welcome Message to LCD
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("WELCOME");
    lcd.setCursor(0, 1);
    lcd.print("Enter a number:");
    // Move to state 1
    state = 1;
  } else if (state == 1) {
    // STATE 1
    // Waiting for user input

    // timeout checker
    if((millis() > start_time+5000) && (selected_char!= '\0'))state = 0;

    // Get input from the keypad
    char key = keypad.getKey();
    
    // Check if user type on the keypad
    if (key == NO_KEY) {
      return;
    }
    // Print the key in the terminal
    Serial.println(key);
    if (key == 'A') {
      // Key A
      if(selected_char != '\0'){
        // If nothing selected
        selected = (int)selected_char - 48;
        if(selected > 4 || selected <1){
          lcd.clear();
          lcd.setCursor(2, 0);
          lcd.print("Please input");
          lcd.setCursor(2, 1);
          lcd.print("Number: 1-4");
          delay(1500);
          state = 0;
          return;
        }
        
        Serial.println(selected);
        state = 2;
      }

    } else if (key == 'B') {
      // Key B
      // Nothing Happen

    } else if (key == 'C') {
      // Key C
      // Reset to state 0
      state = 0;

    } else if (key == 'D') {
      // Key D
      // Resync the data to the server
      while (1) {
        if (syncdata(client)) {
          state = 0;
          break;
        }
      }

    } else {
      // Number Key
      selected_char = key;
      selected = (int)selected_char - 48;
      start_time = millis();
      if(selected > 4 || selected <1){
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("Not Registered");
        lcd.setCursor(2, 1);
        lcd.print("Enter: 1-4");
        delay(1000);
        state = 0;
        return;
      }
      lcd.clear();
      
      if(items[selected-1].getQuantity()>0){
        lcd.setCursor(0, 0);
        lcd.print(items[selected-1].getName());
        lcd.setCursor(12, 0);
        lcd.print("$");
        lcd.print(items[selected-1].getPrice());
      }else{
        lcd.setCursor(2, 0);
        lcd.print("Out of Stock");
        delay(1000);
        state = 0;
      }
      lcd.setCursor(0, 1);
      lcd.print(selected_char);
    }


  } else if (state == 2) {
    // STATE 2
    // Change the text
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Tap card");
    lcd.setCursor(1, 1);
    lcd.print("on the keypad");
    start_time = millis();
    
    state = 3;

  } else if (state == 3) {
    // STATE 3
    // Waiting for user to tap the card

    if(millis() > start_time+10000 ){
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("Timed Out");
      delay(1000);
      state = 0;
    }

    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

    byte block;
    byte len;
    MFRC522::StatusCode status;

    // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if (!mfrc522.PICC_IsNewCardPresent()) {
      return;
    }

    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial()) {
      return;
    }

    Serial.println(F("**Card Detected:**"));

    //-------------------------------------------

    mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid));  //dump some details about the card

    char str[32] = "";
    array_to_string(mfrc522.uid.uidByte, 4, str);  //Insert (byte array, length, char array for output)
    Serial.println(str);                           //Print the output uid string

    Serial.println(F("\n**End Reading**\n"));

    // array_to_string(mfrc522.uid)

    // Stop the card reader from reading 
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    // Call the payment API
    if (payment(client, selected, str) == 1) {
      // If the payment is succeed move to state 4
      delay(500);
      state = 4;
    } else {
      // If the payment is failed, move to initialization state
      delay(1000);
      state = 0;
    }

  } else if (state == 4) {
    // STATE 4
    // Turn on the motor to push the selected item

    switch(selected){
      case 1:
        servo1.write(0);
        delay(time_servo1);
        servo1.write(90);
        break;
      case 2:
        servo2.write(180);
        delay(time_servo2);
        servo2.write(90);
        break;
      case 3:
        servo3.write(0);
        delay(time_servo3);
        servo3.write(90);
        break;
      case 4:
        servo4.write(180);
        delay(time_servo4);
        servo4.write(90);        
        break;
    }
    items[selected-1].decreaseQty();

    delay(1000);
    state = 5;

  } else if (state == 5) {
    // STATE 5
    // Print thank you message and return to the initialization state
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Thank you!");
    delay(1000);
    state = 0;

  }

}
//*****************************************************************************************//
