/*
 * Developed by Denk Development in Apr 2018
 * License: MIT
 */

const int FLOAT_COUNT = 4;
volatile float validFloat[FLOAT_COUNT]; // usable values

const int DATA_PIN = 8;
const int CLOCK_PIN = 2;

const byte END_MARKER = 0B10101010;
const int DATA_LENGTH = sizeof(float) * FLOAT_COUNT; // number of bytes without check-sum and end marker
volatile byte buffer[DATA_LENGTH]; // buffer for incoming data
volatile bool dataInvalid = true, checkValid = false;
volatile byte junk = 0, // byte that is currently being received
  check = 0, // xor-check sum byte
  bitsRemaining = 8, // number of bits that are missing in the current byte
  byteCount = 0; // number of bytes received so far (from a package of length DATA_LENGTH + 2)

void setup() {
  pinMode(DATA_PIN, INPUT);
  pinMode(CLOCK_PIN, INPUT);
  
  initTimerInterrupt();
  attachInterrupt(digitalPinToInterrupt(CLOCK_PIN), dataAvailable, RISING);

  Serial.begin(9600);
}

void loop() {
  // debug logging
  Serial.print(byteCount);
  Serial.print(" - ");
  Serial.print(buffer[0], BIN);
  Serial.print(" ");
  Serial.print(buffer[1], BIN);
  Serial.print(" ");
  Serial.print(buffer[2], BIN);
  Serial.print(" ");
  Serial.print(buffer[3], BIN);
  Serial.print(" - ");
  Serial.print(check, BIN);
  Serial.print(" ");
  Serial.print(validFloat[0]); // output first value
  Serial.print(" ");
  Serial.print(validFloat[1]); // output second value
  Serial.print(" ");
  Serial.print(validFloat[2]); // output third value
  Serial.print(" ");
  Serial.print(validFloat[3]); // output fourth value
  Serial.println();
}

void dataAvailable() {
  junk = (junk << 1) | (PINB & 1);
  bitsRemaining--;
  
  if (!bitsRemaining) { // > 0 check
    // an entire byte was received
    check ^= junk; // update check sum
    bitsRemaining = 8;

    if (junk == END_MARKER && dataInvalid && random(0, 10) < 5) {
      // This if clause is used in case invalid data was transmitted and a
      // random byte equals the END_MARKER.
      // In that case, the receiver resets its values and expects a new transmission to start.
      // The random condition was added because the transmitted data may contain the END_MARKER
      // as well in which case the receiver would continue to be out of sync forever.
      dataInvalid = false;
      byteCount = 255; // overflow to 0 at the end of this function
      check = 0;
      checkValid = false;
    }
    
    if (byteCount < DATA_LENGTH) { // data part of the packet
      buffer[byteCount] = junk;
    }
    if (byteCount == DATA_LENGTH) { // check sum comparison
      checkValid = (check == 0);
    }
    if (byteCount == DATA_LENGTH + 1) { // end marker
      dataInvalid = !(checkValid && junk == END_MARKER);
      if (!dataInvalid) {
        // end marker and check sum are valid
        // copy the received data from the buffer to the target memory
        memcpy(&validFloat, buffer, DATA_LENGTH);
      }
      
      checkValid = false;
      byteCount = 255;
      check = 0; // reset check sum (could have been invalid and != 0)
    }
    
    byteCount++;
  }
  else {
    // reset timer0 counter
    TCNT0 = 0;
  }
}

void initTimerInterrupt() {
  cli(); // disable interrupts

  // timer 0 is being used
  // if needed, timer 1 and 2 would also be suitable for the chosen frequency
  // frequency: 2000 Hz
  
  TCCR0A = 0;
  TCCR0B = 0;
  TCNT0  = 0;
  
  OCR0A = 30; // compare match register
  
  TCCR0B |= (1 << CS02); // set CS02 bit for 1:256 prescaler

  TCCR0A |= (1 << WGM01); // turn on CTC mode
  TIMSK0 |= (1 << OCIE0A); // enable timer compare interrupt
  
  sei(); // allow interrupts
}

ISR(TIMER0_COMPA_vect) {
  // timer0 ISR
  // There was no external interrupt at the clock pin for 500 microseconds
  // therefore it is likely that the current byte shift has ended and the
  // number of remaining bits is 8.
  // If that reset was not made, one glitch at the clock pin would bring
  // transmitter and receiver out of sync.
  
  bitsRemaining = 8;
}
