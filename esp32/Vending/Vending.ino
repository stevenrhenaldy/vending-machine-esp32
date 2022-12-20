/*
 * Uses MIFARE RFID card using RFID-RC522 reader
 * Uses MFRC522 - Library
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout

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
#include <string.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Keypad.h>

const char* ssid = "L";
const char* password = "1234567890";
String host = "http://192.168.0.118/vending/api.php";

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define RST_PIN 4  // Configurable, see typical pin layout above
#define SS_PIN 5   // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

#include <ESP32Servo.h>

int pinServo1 = 17;
int pinServo2 = 16;
int pinServo3 = 2;//15
int pinServo4 = 15;//2

int time_servo1 = 590;
int time_servo2 = 550;
int time_servo3 = 700;
int time_servo4 = 610;

Servo servo1; 
Servo servo2; 
Servo servo3; 
Servo servo4;

const byte rows = 4;
const byte cols = 4;
char keys[rows][cols] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' },
};

byte rowPins[rows] = { 32, 33, 25, 26 };
byte colPins[rows] = { 27, 14, 12, 13 };

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);


class Item {
private:
  int price;
  String name;
  int quantity;

public:
  void setItem(int p, String n, int q) {
    this->price = p;
    this->name = n;
    this->quantity = q;
  }

  void decreaseQty(){
    this->quantity -= 1;
  }

  int getPrice() {
    return this->price;
  }
  String getName() {
    return this->name;
  }
  int getQuantity() {
    return this->quantity;
  }
};

Item items[4];


int syncdata(WiFiClient client) {
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sync Data");
  HTTPClient http;

  String url = host + "?action=sync";
  // Your Domain name with URL path or IP address with path
  http.begin(client, url.c_str());
  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  // Data to send with HTTP POST
  String httpRequestData = "secret=WuPzCYPzvKbQ1mE25HMf&vending_id=1";
  // Send HTTP POST request
  int httpResponseCode = http.POST(httpRequestData);

  String payload = "{}";

  if (httpResponseCode == 200) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR");
    lcd.setCursor(0, 1);
    lcd.print(httpResponseCode);
    return 0;
  }
  // Serial.println(payload);

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, payload);
  items[0].setItem(doc["1"]["price"], doc["1"]["name"], doc["1"]["quantity"]);
  items[1].setItem(doc["2"]["price"], doc["2"]["name"], doc["2"]["quantity"]);
  items[2].setItem(doc["3"]["price"], doc["3"]["name"], doc["3"]["quantity"]);
  items[3].setItem(doc["4"]["price"], doc["4"]["name"], doc["4"]["quantity"]);
  printItemsToLCD(items, 4);
  return 1;
}

int payment(WiFiClient client, int item_num, String card_uid) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Please Wait...");
  HTTPClient http;

  String url = host + "?action=payment";
  // Your Domain name with URL path or IP address with path
  http.begin(client, url.c_str());
  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  // Data to send with HTTP POST
  String httpRequestData = "secret=WuPzCYPzvKbQ1mE25HMf&vending_id=1&card_uid=" + card_uid + "&item_num=" + item_num;
  Serial.println(httpRequestData);
  // Send HTTP POST request
  int httpResponseCode = http.POST(httpRequestData);

  String payload = "{}";

  if (httpResponseCode == 200) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR");
    lcd.setCursor(0, 1);
    lcd.print(httpResponseCode);
    return 0;
  }
  // Serial.println(payload);

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, payload);
  String status = doc["status"];
  Serial.println(status);
  if (status == "success") {
    int balance = doc["balance"];
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Payment Success");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Balance:");
    lcd.setCursor(0, 1);
    lcd.print("$ ");
    lcd.print(balance);
    
    return 1;
  } else if (status == "error") {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR");
    lcd.setCursor(0, 1);
    String msg = doc["message"];
    lcd.print(msg);
    delay(1000);
    return 0;
  }
}

void printItemsToLCD(Item items[], int size) {
  for (int i = 0; i < size; i++) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(i + 1);
    lcd.setCursor(3, 0);
    lcd.print(items[i].getName());
    lcd.setCursor(0, 1);
    lcd.print("$ ");
    lcd.print(items[i].getPrice());
    lcd.setCursor(8, 1);
    lcd.print("Qty: ");
    lcd.print(items[i].getQuantity());
    delay(1000);
  }
}
//*****************************************************************************************//
void setup() {
  Serial.begin(115200);  // Initialize serial communications with the PC
  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522 card

  // The servo control wire is connected to Arduino D2 pin.
  servo1.attach(pinServo1);
  servo2.attach(pinServo2);
  servo3.attach(pinServo3);
  servo4.attach(pinServo4);

  lcd.init();
  lcd.backlight();

  WiFi.begin(ssid, password);
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


  WiFiClient client;

  while (1) {
    if (syncdata(client)) {
      break;
    }
  }

  // Free resources
  // http.end();

  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("WELCOME");
  lcd.setCursor(0, 1);
  lcd.print("Enter a number:");
}

int state = 0;
bool is_num_selected = false;
char selected_char;
int selected;
unsigned long start_time = 0;
//*****************************************************************************************//
void loop() {
  if (state == 0) {
    // Serial.println("State 0");
    selected_char = '\0';
    is_num_selected = false;
    selected = 0;
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("WELCOME");
    lcd.setCursor(0, 1);
    lcd.print("Enter a number:");
    state = 1;
  } else if (state == 1) {
    if((millis() > start_time+5000) && (selected_char!= '\0'))state = 0;
    // Serial.println("State 1");
    char key = keypad.getKey();
    if (key != NO_KEY) {
      Serial.println(key);
    }else{
      return;      
    }

    if (key == 'A') {
      if(selected_char != '\0'){
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
      
    } else if (key == 'C') {
      state = 0;
    } else if (key == 'D') {
      WiFiClient client;
      while (1) {
        if (syncdata(client)) {
          state = 0;
          break;
        }
      }
    } else {
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
    
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Tap card");
    lcd.setCursor(1, 1);
    lcd.print("on the keypad");
    start_time = millis();
    
    state = 3;

  } else if (state == 3) {
    if(millis() > start_time+10000 ){
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("Timed Out");
      delay(1000);
      state = 0;
    } 
    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

    //some variables we need
    byte block;
    byte len;
    MFRC522::StatusCode status;

    //-------------------------------------------

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
    WiFiClient client;

    // array_to_string(mfrc522.uid)

    if (payment(client, selected, str) == 1) {
      delay(500);
      state = 4;
    } else {

      delay(1000);
      state = 0;
    }

    // delay(1000);  //change value if you want to read cards faster
    
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

  } else if (state == 4) {

    if(selected == 1){
      servo1.write(0);
      delay(time_servo1);
      servo1.write(90);
    }else if(selected == 2){
      servo2.write(180);
      delay(time_servo2);
      servo2.write(90);
    }else if(selected == 3){
      servo3.write(0);
      delay(time_servo3);
      servo3.write(90);
    }else if(selected == 4){
      servo4.write(180);
      delay(time_servo4);
      servo4.write(90);
    }
    items[selected-1].decreaseQty();

    delay(1000);
    state = 5;

  } else if (state == 5) {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Thank you!");
    delay(1000);
    state = 0;
  }
}
//*****************************************************************************************//

void array_to_string(byte array[], unsigned int len, char buffer[]) {
  for (unsigned int i = 0; i < len; i++) {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i * 2 + 0] = nib1 < 0xA ? '0' + nib1 : 'A' + nib1 - 0xA;
    buffer[i * 2 + 1] = nib2 < 0xA ? '0' + nib2 : 'A' + nib2 - 0xA;
  }
  buffer[len * 2] = '\0';
}
