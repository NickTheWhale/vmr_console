#include <Bounce2.h>
int x;

const int D_PINS = 27; // number of pins
const int DIGITAL_PINS[D_PINS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 32, 31, 30, 29, 33, 34, 35, 36, 37, 38, 39, 40, 41, 28}; // pins buttons are connected to
const int BOUNCE_TIME = 50;
const int A = 25;
const int B = 26;
const int C = 27;
const int A_PINS = 14;
bool dData[D_PINS] = {0};

String dataLag = "";

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
  Serial.setTimeout(1);

  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  for (int i = 0; i < D_PINS; i++) {
    pinMode(DIGITAL_PINS[i], INPUT_PULLUP);
  }
}
void loop() {
  if (dataLag != getAllData()) {
    dataLag = getAllData();
    Serial.println(getAllData());
  }
}

String getAnalogData() {
  int iterations = 50;
  int data[A_PINS] = {0};
  String dataString;
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
    data[i] = map(data[i], 5, 1023, -60, 12);
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

String getAllData() {
  String dataString;
  dataString = getAnalogData() + ',' + getDigitalData();
  return dataString;
}
