#define BT1 27
#define BT2 26
#define ZD 25
#define RL1 33
#define RL2 32
bool onn = 0;
bool off = 1;

void setup() {
  pinMode(ZD, INPUT);
  pinMode(BT1, INPUT_PULLUP);
  pinMode(BT2, INPUT_PULLUP); 
  pinMode(RL1, OUTPUT);
  pinMode(RL2, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(BT1), van1, RISING);
  attachInterrupt(digitalPinToInterrupt(BT2), van2, RISING);
  attachInterrupt(digitalPinToInterrupt(ZD), check, FALLING);
}

void check(){
  onn = 1;
}

void van1(){
  digitalWrite(RL1, !digitalRead (RL1));
}

void van2(){
  onn = 0;
  off = !off;
}

void loop() {
  if (onn && !off)
    digitalWrite(RL2, HIGH);
  if (onn && off)
    digitalWrite(RL2, LOW);
}
