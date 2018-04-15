/*
 * Developed by Denk Development in Apr 2018
 * License: MIT
 */

const int DATA_PIN = 3;
const int CLOCK_PIN = 4;
const byte END_MARKER = 0B10101010;
const int FLOAT_COUNT = 4;

void setup() {
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Serial.begin(9600);
}

void loop() {
  float x[FLOAT_COUNT];
  x[0] = 3.14;
  x[1] = 4.15;
  while (x) {
    sendData((byte *) &x, sizeof(float) * FLOAT_COUNT);
    delay(1000);
    x[0] += 1;
    x[1] -= 1;
  }
}

void sendData(byte *d, int len) {
  byte check = 0;
  for (int i = 0; i < len; i++) {
    byte junk = d[i];
    shift(junk);
    check ^= junk;
    delay(1);
  }
  shift(check);
  delay(1);
  shift(END_MARKER);
  delay(2);
  Serial.println();
}

void shift(byte d) {
  Serial.print(d, BIN);
  Serial.print(" ");
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, d);
}
