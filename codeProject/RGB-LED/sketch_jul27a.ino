/*
   RGB LED
*/
int redPin = 11;    // the pin that the red LED is attached to
int greenPin = 10; // the pin that the green LED is attached to
int bluePin = 9;  // the pin that the blue LED is attached to
void setup(){
     pinMode(redPin, OUTPUT);
     pinMode(greenPin, OUTPUT);
     pinMode(bluePin, OUTPUT);
}

void loop(){
    colorRGB(0, 255, 0);    // Xanh la
    delay(2000);
    colorRGB(0, 0, 255);    // Xanh duong
    delay(2000);
    colorRGB(255, 0, 0);    // Do
    delay(2000);
    colorRGB(255, 255, 255);  // Trang
    delay(2000);
    colorRGB(255, 20, 0);     // Vang
    delay(2000);
    colorRGB(255, 0, 255);    // Tim
    delay(2000);
    
}

void colorRGB(int i, int j, int k){
     analogWrite(redPin, i);
     analogWrite(greenPin, j);
     analogWrite(bluePin, k);
}
