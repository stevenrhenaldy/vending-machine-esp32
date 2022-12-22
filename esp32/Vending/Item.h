#include <Arduino.h>

class Item {
private:
  int price;
  String name;
  int quantity;

public:
  void setItem(int p, String n, int q);
  void decreaseQty();
  int getPrice();
  String getName();
  int getQuantity();
};