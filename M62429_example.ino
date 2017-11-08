/*
  Serial controlled volume based on this sketch: http://forum.arduino.cc/index.php?topic=244152.0
  The above sketch is also based on this lib: https://github.com/krupski/M62429/blob/master/M62429.cpp
*/

int VolumePinDAT = 8; // connect to DATA
int VolumePinCLK = 14; // connect to CLOCK
int vol = 0;

void setup (void) {
  pinMode(VolumePinDAT, OUTPUT);
  pinMode(VolumePinCLK, OUTPUT);

  Serial.begin(115200);
  while (!Serial);
  Serial.println("Ready.");
}

void loop (void) {
  if (Serial.available() > 0) {
    int ch = Serial.read();

    // Store received digit.
    if (ch >= '0' && ch <= '9') {
      vol = vol * 10 + (ch - '0');
    } else if (ch == 0x0a) {
      // When receive LF, check range of vol.
      if (vol >= 0 && vol <= 100) {
        // If vol is in valid range, set sent it to M62429.
        Serial.print("Set volume:");
        setVolume(vol);
      } else {
        Serial.print("Out of range:");
      }
      Serial.println(vol, DEC);
      vol = 0;
    }
  }
}

// this function does the job
void setVolume (uint8_t volume) {
  uint8_t bits;
  uint16_t data = 0; // control word is built by OR-ing in the bits

  // convert attenuation to volume
  volume = (volume > 100) ? 0 : (((volume * 83) / -100) + 83); // remember 0 is full volume!
  // generate 10 bits of data
  data |= (0 << 0); // D0 (channel select: 0=ch1, 1=ch2)
  data |= (0 << 1); // D1 (individual/both select: 0=both, 1=individual)
  data |= ((21 - (volume / 4)) << 2); // D2...D6 (ATT1: coarse attenuator: 0,-4dB,-8dB, etc.. steps of 4dB)
  data |= ((3 - (volume % 4)) << 7); // D7...D8 (ATT2: fine attenuator: 0...-1dB... steps of 1dB)
  data |= (0b11 << 9); // D9...D10 // D9 & D10 must both be 1

  for (bits = 0; bits < 11; bits++) { // send out 11 control bits
    delayMicroseconds (2); // pg.4 - M62429P/FP datasheet
    digitalWrite (VolumePinDAT, 0);
    delayMicroseconds (2);
    digitalWrite (VolumePinCLK, 0);
    delayMicroseconds (2);
    digitalWrite (VolumePinDAT, (data >> bits) & 0x01);
    delayMicroseconds (2);
    digitalWrite (VolumePinCLK, 1);
  }
  delayMicroseconds (2);
  digitalWrite (VolumePinDAT, 1); // final clock latches data in
  delayMicroseconds (2);
  digitalWrite (VolumePinCLK, 0);
  //return data; // return bit pattern in case you want it :)
}

