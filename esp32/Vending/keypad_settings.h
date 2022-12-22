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