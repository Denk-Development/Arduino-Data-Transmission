# Arduino Data Transmission
Secured one-way data transmission between Arduinos. The sample snippets transmit an array of float values and use an xor checksum to validate the correctness of the received data. The focus of this piece of software is to ensure the validity of every single received value. Any interruption of clock or data connection should not result in wrong values that the receiver considers valid.

## Wiring
| Function | Transmitter | Receiver |
| --- | --- | --- |
| Clock | 4 | 2 |
| Data | 3 | 8 |
| Ground | GND | GND |

Changing the receiver clock pins requires adjustment of the ISR which uses port manipulation to read the data pin. Furthermore, the transmitter clock pin must support external interrupts.

## Logging
For debugging and testing purposes, both transmitter and receiver log their activities.
