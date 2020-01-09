int buzzerPin=9;

void setup() {
 pinMode(buzzerPin, OUTPUT);
 }

void loop() {
  ringTone(buzzerPin);   
  }

void ringTone(int pin) {
  for (int i=0; i<10; i++) { //repeat 10 times
    tone(pin, 1000);
    delay(50);
    tone(pin, 500);
    delay(50);
    }
  noTone(pin);
  delay(2000);
  } b
