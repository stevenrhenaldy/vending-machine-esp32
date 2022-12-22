void array_to_string(byte array[], unsigned int len, char buffer[]) {
  for (unsigned int i = 0; i < len; i++) {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i * 2 + 0] = nib1 < 0xA ? '0' + nib1 : 'A' + nib1 - 0xA;
    buffer[i * 2 + 1] = nib2 < 0xA ? '0' + nib2 : 'A' + nib2 - 0xA;
  }
  buffer[len * 2] = '\0';
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