const int whiteLEDPin = 33;
const int redLEDPin = 32;
const int buttonPin = 27;

void setup() {

  pinMode(whiteLEDPin, OUTPUT);
  pinMode(redLEDPin, OUTPUT);
  

  pinMode(buttonPin, INPUT_PULLUP); // High except button is pushed: Low
}

void loop() {

  int buttonState = digitalRead(buttonPin);


  if (buttonState == LOW) {
    digitalWrite(whiteLEDPin, HIGH); // turn on white LED
    digitalWrite(redLEDPin, HIGH);   // turn on red LED
  } else {
    digitalWrite(whiteLEDPin, LOW);  // turn off white LED
    digitalWrite(redLEDPin, LOW);    // turn off red LED
  }
}
