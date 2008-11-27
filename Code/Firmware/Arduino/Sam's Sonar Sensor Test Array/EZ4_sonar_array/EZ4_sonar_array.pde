/*
 * AnalogInput
 * by Sam Misko
 *
 * Takes in the DC analog values from the 4 sonar sensors, and if they are 
 * less than what corresponds to 5", then the corresponding LED will light up.
 
 */
                    //select analog input pins
int sonarOne = 0;
int sonarTwo = 1;
int sonarThree = 2;
int sonarFour = 3;

                    // select the pins for the LEDs
int ledPin1 = 8;
int ledPin2 = 7;
int ledPin3 = 6;
int ledPin4 = 5;

                    // variables to store the values coming from the sensors
int readingOne = 0;
int readingTwo = 0;
int readingThree = 0;
int readingFour = 0;

void setup() {
   
  Serial.begin(9600); 

                    //declare the ledPin1-4 as OUTPUT          
  pinMode(ledPin1, OUTPUT);  
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin4, OUTPUT);
}

void loop() {
                    // read the values from the sensors
  readingOne = analogRead(sonarOne);
  readingTwo = analogRead(sonarTwo);
  readingThree = analogRead(sonarThree);
  readingFour = analogRead(sonarFour);
  
                    // check the value returned from the sensor to see if there is an object detected;
                    // if there is an object present, then light the apporpriate LED
                    // if there is not and object present, then turn off the appropriate LED
  if (readingOne < 20) {
    digitalWrite(ledPin1, HIGH);                         
   }
   else {
     digitalWrite(ledPin1, LOW);
   }
  if (readingTwo < 20) {
    digitalWrite(ledPin2, HIGH);                         
   }
   else {
     digitalWrite(ledPin2, LOW);
   }
  if (readingThree < 20){
    digitalWrite(ledPin3, HIGH);
  }
  else {
     digitalWrite(ledPin3, LOW);
   }
  if (readingFour < 20) {
    digitalWrite(ledPin4, HIGH);
  }
  else {
     digitalWrite(ledPin4, LOW);
   }
  
  
  
  
                   //display sensor readings on screen
  Serial.print("readingOne:");
  Serial.println(readingOne);
  delay(100);
  Serial.print("readingTwo:");
  Serial.println(readingTwo);
  delay(100);
  Serial.print("readingThree:");
  Serial.println(readingThree);
  delay(100);
  Serial.print("readingFour:");
  Serial.println(readingFour);
  delay(100);
  
}
