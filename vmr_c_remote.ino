#include <Bounce2.h>

const int D_PINS = 27; // number of pins  13
const int DIGITAL_PINS[D_PINS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41}; // pins buttons are connected to
const int BOUNCE_TIME = 50;
const int A = 25;
const int B = 26;
const int C = 27;
const int A_PINS = 14;
bool dData[D_PINS] = {0};
#define PRIME_A 54059 /* a prime */
#define PRIME_B 76963 /* another prime */
#define PRIME_C 86969 /* yet another prime */
#define FIRSTH 37 /* also prime */

String dataLag = "";
unsigned long previousTime;

#define MAX_MESSAGE_LENGTH 255

#define WAIT_FOR_RUN false
#define ITERATIONS 50           //number of analog readings
#define INTERVAL 10             //interval in milliseconds to send data

Bounce digital[] =   {
  Bounce(DIGITAL_PINS[0], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[1], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[2], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[3], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[4], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[5], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[6], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[7], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[8], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[9], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[10], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[11], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[12], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[13], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[14], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[15], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[16], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[17], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[18], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[19], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[20], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[21], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[22], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[23], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[24], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[25], BOUNCE_TIME),
  Bounce(DIGITAL_PINS[26], BOUNCE_TIME),
};

void setup() {
  Serial.begin(115200);

  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  for (int i = 0; i < D_PINS; i++) {
    pinMode(DIGITAL_PINS[i], INPUT_PULLUP);
  }

  bool consoleReady = false;
  String incomingMessage;
  if (WAIT_FOR_RUN) {
    while (!consoleReady) {
      if (Serial.available() > 0) {
        incomingMessage = Serial.read();
        if (incomingMessage == "r") {
          consoleReady = true;
        }
      }
      Serial.flush();
    }
    Serial.flush();
  }
  previousTime = millis();
}

void loop() {
  String dataBuffer = "";
  dataBuffer = getAllData(true);
  if (dataLag != dataBuffer) {
    dataLag = dataBuffer;
    Serial.print(dataBuffer);
    Serial.println();
  }

  //  if (millis() - INTERVAL > previousTime){
  //    Serial.print(dataBuffer);
  //    Serial.println();
  //  }
}


String getAnalogData(bool knobs) {
  int iterations = ITERATIONS;
  int data[A_PINS] = {0};
  String dataString;
  if (knobs) {
    for (int i = 0; i < iterations; i++) {
      for (int i = 0; i < 8; i++) {
        digitalWrite(A, HIGH && (i & B00000001));
        digitalWrite(B, HIGH && (i & B00000010));
        digitalWrite(C, HIGH && (i & B00000100));
        data[i] += analogRead(A0);
      }
    }
    for (int i = 0; i < iterations; i++) {
      for (int i = 0; i < 6; i++) {
        digitalWrite(A, HIGH && (i & B00000001));
        digitalWrite(B, HIGH && (i & B00000010));
        digitalWrite(C, HIGH && (i & B00000100));
        data[i + 8] += analogRead(A1);
      }
    }

    for (int i = 0; i < A_PINS; i++) {
      data[i] /= iterations;
      data[i] = map(data[i], 20, 1010, -60, 12);
      data[i] = constrain(data[i], -60, 12);
    }
  }
  else if (!knobs)
  {
    for (int i = 0; i < iterations; i++) {
      for (int i = 0; i < 6; i++) {
        digitalWrite(A, HIGH && (i & B00000001));
        digitalWrite(B, HIGH && (i & B00000010));
        digitalWrite(C, HIGH && (i & B00000100));
        data[i + 8] += analogRead(A1);
      }
    }

    for (int i = 0; i < A_PINS; i++) {
      data[i] /= iterations;
      data[i] = map(data[i], 20, 1010, -60, 12);
      data[i] = constrain(data[i], -60, 12);
    }
  }

  for (int i = 0; i < A_PINS; i++) {
    //int mapVal = (( 1.6 * data[13] ) - .0047 * (data[13] * data[13]));
    int mapVal = data[13];
    if (i != 13) {
      dataString += String(data[i]);
      dataString += ',';
    }
    else {
      dataString += String(mapVal);
    }
  }

  return dataString;
}

String getDigitalData() {
  String dataString;
  for (int i = 0; i < D_PINS; i++) {
    digital[i].update();
    if (digital[i].fallingEdge()) {
      dData[i] = !dData[i];
    }
  }
  for (int i = 0; i < D_PINS - 1; i++) {
    dataString += String(dData[i]);
    dataString += ',';
  }
  dataString += String(dData[D_PINS - 1]);
  return dataString;
}

String getAllData(bool knobs) {
  String dataString;
  String dataCopy;
  unsigned hash;
  dataString = getAnalogData(knobs) + ',' + getDigitalData();
  dataCopy = dataString;
  const char *c = dataCopy.c_str();
  hash = getHash(c);

  return '<' + String(hash) + "," + dataString + '>';
}

unsigned getHash(const char* s) {
  unsigned h = FIRSTH;
  while (*s) {
    h = (h * PRIME_A) ^ (s[0] * PRIME_B);
    s++;
  }
  return h % PRIME_C; // or return h % PRIME_C;
}

void receiveData() {
  if (Serial.available() > 0) {
    static char message[MAX_MESSAGE_LENGTH];
    static unsigned int message_pos = 0;
    char inByte = Serial.read();
    if ( inByte != '\n' && (message_pos < MAX_MESSAGE_LENGTH - 1) ) {
      message[message_pos] = inByte;
      message_pos++;
    }
    else {
      message[message_pos] = '\0';
      Serial.println(message);
      message_pos = 0;
    }
  }
  Serial.flush();
}
