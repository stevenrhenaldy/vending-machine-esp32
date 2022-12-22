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
  String httpRequestData = "secret="+api_key+"&vending_id="+vending_id;
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
  http.begin(client, url.c_str());
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String httpRequestData = "secret="+api_key+"&vending_id="+vending_id+"&card_uid=" + card_uid + "&item_num=" + item_num;

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
    delay(500);
    return 0;
  }

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