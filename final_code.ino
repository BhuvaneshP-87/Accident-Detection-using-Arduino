
// include the library code:
#include <LiquidCrystal.h> //LCD displaY
#include <Wire.h>     // Including the wire library
#include <MPU6050.h> // Including the MPU6050 library
#include <SoftwareSerial.h>

long distance=0,pitch_value=0,roll_value=0;
double latitude=0.0,longitude=0.0;
const int min_distance=5,thres_pitch=50,thres_roll=50,warn_distance=30,warn_pitch=30,warn_roll=30;
const int pingPin = 4; // Trigger Pin of Ultrasonic Sensor
const int echoPin = 5; // Echo Pin of Ultrasonic Sensor
const int rs = 13, en = 12, d4 = 8, d5 = 9, d6 = 10, d7 = 11; //lcd pins

SoftwareSerial mySerial(3, 2);   //Create software serial object to communicate with SIM800L
MPU6050 accelerometer;  // Initializing a variable name accelerometer of type MPU6050
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); // initialize the library by associating any needed LCD interface pin


void setup() {
  // set up serial monitor to output values
  Serial.begin(9600);
  //Begin serial communication with Arduino and SIM800L
  mySerial.begin(9600);
  
  Serial.println(F("Initializing the Accelerometer MPU6050 sensor. Wait for a while"));
  delay(2000);
  while (!accelerometer.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G)) // Checking whether the mpu6050 is sensing or not
  {
    Serial.println(F("Failed to read from the sensor. Something is Wrong!"));
    delay(500);
  }
  Serial.println(F("Initializing the Ultrasonic sensor. Wait for a while"));
  delay(2000);
  Serial.println(F("Initializing the LCD Display. Wait for a while"));
  delay(2000);
  Serial.println(F("Initializing the GSM/GPRS Sensor. Wait for a while"));
  delay(2000);
  lcd.begin(16, 2);
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
   getDistance();
   getAccelerometer();
   lcd.setCursor(0, 0);
   lcd.clear();
   lcd.print("d:");
   lcd.print(distance);
   lcd.print("cm p:");
   lcd.print(pitch_value);
   lcd.print(" r:");
   lcd.print(roll_value);
   Serial.print(F("Distance = "));
   Serial.print(distance);
   Serial.print(F(" Pitch = "));
   Serial.print(pitch_value);
   Serial.print(F(" Roll = "));
   Serial.print(roll_value);
   if( distance<min_distance || pitch_value>thres_pitch || pitch_value<-(thres_pitch) || roll_value>thres_roll || roll_value<-(thres_roll)) {
    lcd.setCursor(0, 1);
    lcd.print("CRASHED");
    sendSMS();
    for(;;){
      
    }
   }
   else{ 
    if( distance<warn_distance || pitch_value>warn_pitch || pitch_value<-(warn_pitch) || roll_value>warn_roll || roll_value<-(warn_roll)) {
      lcd.setCursor(0, 1);
      lcd.print("WARNING");
    }
    else{
      lcd.setCursor(0, 1);
      lcd.print("Good Going");
    }
   }
   Serial.println();
   delay(2000);
}

void getDistance(){
   long duration;
   pinMode(pingPin, OUTPUT);
   digitalWrite(pingPin, LOW);
   delayMicroseconds(2);
   digitalWrite(pingPin, HIGH);
   delayMicroseconds(10);
   digitalWrite(pingPin, LOW);
   pinMode(echoPin, INPUT);
   duration = pulseIn(echoPin, HIGH);
   distance = microsecondsToCentimeters(duration);
}
long microsecondsToCentimeters(long microseconds) {
   return microseconds / 29 / 2;
}


void getAccelerometer()
{
  Vector sensor_data = accelerometer.readNormalizeAccel();    // Reading the acceleration values from the sensor
  pitch_value = -(atan2(sensor_data.XAxis, sqrt(sensor_data.YAxis * sensor_data.YAxis + sensor_data.ZAxis * sensor_data.ZAxis)) * 180.0) / M_PI; // Calculating the pitch value
  roll_value = (atan2(sensor_data.YAxis, sensor_data.ZAxis) * 180.0) / M_PI; // Calculating the Raw value
}

void sendSMS()
{
  Serial.println(F("Initializing GSM/GPRS module"));
  delay(5000);
  lcd.clear();
  lcd.print("Sending Alert");
  Serial.println(F("Enabling Echo"));
  mySerial.println("ATE1"); //Enable Echo if not enabled by default 
  delay(2000);
  Serial.println(F("Handshake Test "));
  mySerial.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();
  delay(2000);
  Serial.println(F("Signal Quality Check"));
  mySerial.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
  updateSerial();
  Serial.println(F("Sim Information"));
  mySerial.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
  updateSerial();
  delay(2000);
  Serial.println(F("Network Check"));
  mySerial.println("AT+CREG?"); //Check whether it has registered in the network
  updateSerial();
  delay(2000);
  Serial.println(F("Giving Missed Call to the number"));  
  mySerial.println("ATD+918904889694;");// Call the number
  updateSerial();
  delay(4000);
  mySerial.println("ATH");// Hang up the call
  updateSerial();
  delay(1000);
  Serial.println(F("Turning on GPROS"));
  mySerial.println("AT+CGATT=1"); // Connect SIM to GPROS
  updateSerial();
  delay(1000);
  Serial.println(F("Clearing Previous Connection"));
  mySerial.println("AT+SAPBR=0,1");// Clear connection
  updateSerial();
  delay(1000);
  Serial.println(F("Fetching Location"));
  mySerial.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""); // Activate bearer profile with connection type GPRS
  updateSerial();
  delay(1000);
  mySerial.println("AT+SAPBR=3,1,\"APN\",\"www\"");// Set VPN for bearer Profile
  updateSerial();
  delay(1000);
  mySerial.println("AT+SAPBR=1,1");// Open Bearer profile
  updateSerial();
  delay(1000);
  mySerial.println("AT+SAPBR=2,1");//Get the IP address of the bearer profile
  updateSerial();
  delay(1000);
  mySerial.println("AT+CIPGSMLOC=1,1");// Request for location Pincode, latitude and longitude
  updateSerial();
  delay(5000);
  latitude=12.9470667;
  longitude=77.5395956;
  Serial.print(F("latitude : "));
  Serial.print(latitude);
  Serial.print(F(" longitude :"));
  Serial.println(longitude);
  
  Serial.println(F("SMS process"));
  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  delay(2000);
  mySerial.println("AT+CMGS=\"+918904889694\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
  updateSerial();
  delay(3000);
  mySerial.print("Accident has occured at https://www.google.com/maps/search/?api=1&query=");
  mySerial.print(latitude);
  mySerial.print(",");
  mySerial.print(longitude);
  updateSerial();
  delay(2000);
  mySerial.write((char)26);
  delay(1000);
  lcd.clear();
  lcd.print("Alert Sent");
  lcd.setCursor(0, 1);
  lcd.print("Successfully");
}

void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(mySerial.available()) 
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}
