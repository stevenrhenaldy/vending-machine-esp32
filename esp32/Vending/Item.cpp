#include "Item.h"

void Item::setItem(int p, String n, int q) {
  this->price = p;
  this->name = n;
  this->quantity = q;
}

void Item::decreaseQty() {
  this->quantity -= 1;
}

int Item::getPrice() {
  return this->price;
}
String Item::getName() {
  return this->name;
}
int Item::getQuantity() {
  return this->quantity;
}