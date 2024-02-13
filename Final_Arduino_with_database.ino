#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <Keypad.h>
#include <LiquidCrystal.h>
#include <ESP32Servo.h>
#include <FirebaseESP32.h>
#include <Wire.h>
#define BUZZER_PIN 1  //TX0
#define GAS_SENSOR_PIN 34
//#define LDR_PIN 15           // LDR analog input pin
//#define LED_PIN_GND1 1      //LED for GND_1
#define LED_PIN_GND2 11      //LED for GND_2
#define ledpin 2 
#define IR_SENSOR_PIN 35
#define SERVO_PIN 17
#define SERVO_PIN_R1 15 //servo for window in the Room
#define RAIN_SENSOR_PIN 4  //connecting it to any digital pin of ESP32 (GPIO)
//#include<wire.h>
//#include <ESP32Servo.h>
//#define GAS_THRESHOLD 500 // Gas concentration threshold (adjust as needed)
const int darknessThreshold = 300; // Adjust this threshold value based on LDR sensitivity
const byte ROWS = 4;
const byte COLS = 4;
//const int servoPin = 10;
bool ledStatus = false;
bool doorStatus = false;
bool winStatus= false;
FirebaseData firebaseData;
Servo doorServo; // Change this to ESP32Servo
Servo winServo;
#define servopin 17 // Define the servo pin number
#define servoWINpin 15
#define WIFI_SSID "iPhone"
#define WIFI_PASSWORD "0453511015"
#define API_KEY "AIzaSyAwqifwV1QOhkjeIli6rj_as-bZSbsekqg"
#define DB_URL "https://final-project-a1b57-default-rtdb.firebaseio.com/"
#define FIREBASE_HOST "final-project-a1b57-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "jfL4jlIU2LtfMn716KgLAQOVO70pI1wkixDcGFd7"

#define USER_EMAIL "1@2.com"

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {27,14,12,13};   // Replace with your keypad pins
byte colPins[COLS] = {26,25,33,32};   // Replace with your keypad pins

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

Firebase_ESP_Client firebase;

Servo doorServo; // Initialize servo motor for Home locked Door
Servo servo1; //Initialize servo motor that connected to Room_1 
LiquidCrystal lcd(18,5,19,21,23,22); // Initialize LCD display

unsigned long sendDataPreMillis = 0;
bool signupOK = false;
int irValue = 0;
float voltage = 0.0;
int rain_value = 0;
int gasValue = 0;
int hack = 1;
int notHack = 0;

FirebaseData fbdo;
FirebaseAuth auth;

FirebaseConfig config;

int pwmValue = 0 ; 
bool ledStatus = false ; 
int count = 0;


String keypadBuffer;
const char* correctPassword = "123456";  // Replace with your desired password
int incorrectAttempts = 0;
int col_num = 0;
bool applicationRunning = true;
//bool isPerson = false;
int pos = 0;
bool isOpen = false;
const int maxDistance = 10;
bool windowOpen = true;// Adjust the distance threshold as needed
//bool isPerson = false;
const int OPEN_ANGLE = 30; // Angle to open the door 
const int CLOSE_ANGLE = 130; // Angle to close the door 
const int OPEN_WIN=90;
const int CLOSE_WIN=0;
void setup() {
  Serial.begin(115200);
  lcd.begin(16, 2);  // Initialize the LCD

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP()); 
  doorServo.attach(servopin); // Attach the servo to the specified GPIO pin
  pinMode(ledpin,OUTPUT);
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.setBool(firebaseData, "/LED_STATUS", ledStatus);
  Firebase.setBool(firebaseData, "/door_STATUS", doorStatus);

  String password = "123456";

  config.api_key = API_KEY;
  config.database_url = DB_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = password;

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  signupOK = true;
  
  
  lcd.print("Enter password:");
  pinMode(IR_SENSOR_PIN, INPUT); // Set the IR pin as an input to read IR sensor data
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GAS_SENSOR_PIN, INPUT);
  pinMode(RAIN_SENSOR_PIN, INPUT);
  //pinMode(LDR_PIN, INPUT); //configuring LDR pin as an input
//  pinMode(LED_PIN_GND1, OUTPUT);   //Led for GND_1
//  pinMode(LED_PIN_GND2, OUTPUT);   //Led for GND_2 

//  doorServo.attach(SERVO_PIN_R1);  
//  doorServo.write(0);  // Initial position (door closed)
  //Attaching servo of Room_1 to its pin
   doorServo.attach(SERVO_PIN);   // Attach the servo object to the servo pin
   doorServo.write(130);
   servo1.attach(SERVO_PIN_R1);
   servo1.write(90); //open the window of Room_1
  
  
}



void loop() {

  if (millis() - sendDataPreMillis > 5000 && Firebase.ready()) {
    sendDataPreMillis = millis();
    Serial.println(auth.token.uid.c_str());
    Serial.println("---------------------");
  }
    if (Firebase.getBool(firebaseData, "/users/hoom door/hoom door")) {
    bool Status = firebaseData.boolData();

    if (Status) {
      Serial.println("Door opened!");
      openDoor();
    } else {
      Serial.println("Door closed!");
      closeDoor();
    }
  }
   if (Firebase.getBool(firebaseData, "/users/lights/lights")) {
    bool Status = firebaseData.boolData();
    
    if (Status) {
      Serial.println("Led Turned ON");
      digitalWrite(ledpin, HIGH);
    } else {
      Serial.println("Led Turned OFF");
      digitalWrite(ledpin, LOW);
    }
  }
  if (Firebase.getBool(firebaseData, "/users/window/window")) {
    bool Status = firebaseData.boolData();
    
     if (Status) {
      Serial.println("Window opened!");
      openWIN();
    } else {
      Serial.println("Window closed!");
      closeWIN();
    }
  }

    if (Firebase.getBool(firebaseData, "/control/hoom door/value")) {
    bool Status = firebaseData.boolData();

    if (Status) {
      Serial.println("Door opened!");
      openDoor();
    } else {
      Serial.println("Door closed!");
      closeDoor();
    }
  }
   if (Firebase.getBool(firebaseData, "/control/lights/value")) {
    bool Status = firebaseData.boolData();
    
    if (Status) {
      Serial.println("Led Turned ON");
      digitalWrite(ledpin, HIGH);
    } else {
      Serial.println("Led Turned OFF");
      digitalWrite(ledpin, LOW);
    }
  }
  if (Firebase.getBool(firebaseData, "/control/window/value")) {
    bool Status = firebaseData.boolData();
    
     if (Status) {
      Serial.println("Window opened!");
      openWIN();
    } else {
      Serial.println("Window closed!");
      closeWIN();
    }
  }
  if (!applicationRunning) {
    return;  // Stop the application if hacker detected
  }
  char key = keypad.getKey();
  if (key) {
    lcd.setCursor(col_num,1);
    lcd.print(key);  // Display an asterisk for each entered digit
    col_num++ ;
    if (key == '#') {
      lcd.clear();
      String enteredPassword = keypadBuffer;

  if ((enteredPassword == correctPassword) ) {
        lcd.setCursor(0, 0);
        lcd.print("Password correct");
        lcd.setCursor(0,1);
        lcd.print("You are Welcome");
        if (Firebase.RTDB.setInt(&fbdo, "notifications/hack", notHack)) {
      Serial.print("there is a hacker ");
      //Serial.println(voltage);
    } else {
      Serial.println("Failed to save voltage data to Firebase");
    }
        
        //delay(1000);
         irValue = analogRead(IR_SENSOR_PIN); // Read the state of the IR sensor (LOW or HIGH)
         voltage = (float)analogReadMilliVolts(IR_SENSOR_PIN) / 1000;
       // Map the IR value from the range [0, 1023] to [0, 10]
     Serial.println(irValue);
     int IRmappedValue = map(irValue, 0, 4095, 0, 10);
     Serial.println(IRmappedValue);
        
        if(!isOpen && (IRmappedValue < 6)){
         // doorServo.write(30);
         lcd.clear();
         lcd.print("Person Detected");
         doorServo.write(30); //open the door
         lcd.setCursor(0,1);
         lcd.print("Door opened..");
         
        delay(2000);
        doorServo.write(130); //close the door
        
// Open the door
        //delay(4000);  // Keep the door open for 4 seconds
        //doorServo.write(130);  // Close the door
        lcd.clear();
        lcd.print("Door closed");
        delay(500);
}else {
    lcd.clear();
    lcd.print("No motion ");
    lcd.setCursor(0,1);
    lcd.print("detected");
    doorServo.write(130);
    delay(1000);
  }
  
  /////////////////////////////////
           //Reading Gas sensor value
  gasValue = analogRead(GAS_SENSOR_PIN); // Read gas sensor value
    
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Gas: ");
  lcd.print(gasValue);
  if (gasValue > 250) {
    lcd.setCursor(0, 1);
    lcd.print("Fire!");
   digitalWrite(BUZZER_PIN, HIGH);
   delay(3000);
   digitalWrite(BUZZER_PIN, LOW);
   // Activate buzzer and LED for gas alert
    //digitalWrite(LED_PIN, HIGH);
  } else {
digitalWrite(BUZZER_PIN, LOW);    //digitalWrite(LED_PIN, LOW);
    lcd.setCursor(0, 1);
    lcd.print("NO Fire");
    }
  delay(2000);
  lcd.clear();
  ////////////////////////////
  //Reading Rain sensor value
  rain_value = digitalRead(RAIN_SENSOR_PIN);
    
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Rain(0/1): ");
  lcd.print(rain_value);
  if (rain_value == 0 && windowOpen){ //Rain Detected
    lcd.setCursor(0, 1);
    lcd.print(" Rain");
    //tone(BUZZER_PIN, 1000); // Activate buzzer for rain alert
    servo1.write(0);
    windowOpen=false;// Move the servo to 0 degrees (fully counterclockwise position)
  }else{
     //noTone(BUZZER_PIN); //this means that rain_value is equal to 1 ==> No rain Detected
     //servo1.write(180);
     //lcd.clear();
     lcd.setCursor(0,1);
     lcd.print("no Rain Detected");
     servo1.write(90); //close the Room_1 window
     
     
  } 
        //delay(1500);


      //Firebaseeeeee   
        if (Firebase.ready() && signupOK && (millis() - sendDataPreMillis > 5000 || sendDataPreMillis == 0)) {
                sendDataPreMillis = millis();


          
        //saving IR sensor readings on on Realtime Database
     if (Firebase.RTDB.setInt(&fbdo, "Sensor/ldr_data", irValue)) {
      Serial.print("IR readings  saved to Firebase: ");
      Serial.println(irValue);
    } else {
      Serial.println("Failed to save IR Sensor readings data to Firebase");
    }

    delay(1000);
    
    //Saving Voltage readings on Realtime database
    if (Firebase.RTDB.setFloat(&fbdo, "Sensor/voltage", voltage)) {
      Serial.print("Voltage data saved to Firebase: ");
      Serial.println(voltage);
    } else {
      Serial.println("Failed to save voltage data to Firebase");
    }
    



     //Saving gas sensor readings on Realtime database
  if (Firebase.RTDB.setInt(&fbdo, "Sensor/gas_data", gasValue)) {
      Serial.print("Gas data saved to Firebase: ");
      Serial.println(gasValue);
    } else {
      Serial.println("Failed to save gas data to Firebase");
    }
    //Saving Rain sensor readings on Realtime database
  if (Firebase.RTDB.setInt(&fbdo, "Sensor/rain_data", rain_value)) {
      Serial.print("Rain data saved to Firebase: ");
      Serial.println(rain_value);
    } else {
      Serial.println("Failed to save rain data to Firebase");
    }
    
  
 }       
   } else {
        incorrectAttempts++;
        if (incorrectAttempts >= 3) {
          lcd.clear();
          lcd.print("This is a hacker");
          
          //Saving Voltage readings on Realtime database
    if (Firebase.RTDB.setInt(&fbdo, "notifications/hack", hack)) {
      Serial.print("there is a hacker ");
      //Serial.println(voltage);
    } else {
      Serial.println("Failed to print");
    }
          
          applicationRunning = false;  // Stop the application
          //tone(BUZZER_PIN, 1000); // Activate buzzer
        } else {
          lcd.setCursor(0, 0);
          lcd.print("Incorrect!");
          lcd.setCursor(0, 1);
          lcd.print("Attempts: ");
          lcd.print(incorrectAttempts);
        }
      }
    

      delay(1000);  // Display the message for 2 seconds
      lcd.clear();
      lcd.print("Enter password:");
      keypadBuffer = "";  // Clear the keypad buffer
      col_num = 0;
    } else {
      keypadBuffer += key;
    }
  }
}
void openDoor() {
  doorServo.write(OPEN_ANGLE); // Open the door
  delay(2000); // Adjust as needed
}

void closeDoor() {
  doorServo.write(CLOSE_ANGLE); // Close the door
  delay(2000); // Adjust as needed
}
void openWIN() {
  winServo.write(OPEN_WIN); // Open the door
  delay(2000); // Adjust as needed
}