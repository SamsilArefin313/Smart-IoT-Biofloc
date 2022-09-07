//==Iot base Bioflok monitoring Monitoring System.===
// Project name: IoT_bioflok.ino
// Supervisor : Fahmida Sharmin Jui  
// Assistant Professor, Dept. of C&E, SUB
// Programing Team :
// 1. Md : Nazim Uddin  ID:008-15-87 
// 2. Md : Mizanur Rahman ID:008-15-89
// Structured: from 10 April 2021
// Modified by: emilon@gmail.com
// Modified from: 12 April 2021
// Contact: +8801711686161
//HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
//  ## Hardware Connections (Breakoutboard to ESP32 Arduino):
//  -VIN = 3.3V
//  -GND = GND
//  -SDA = 21 (or SDA)
//  -SCL = 22 (or SCL)
//  -INT = Not connected
#include <DFRobot_ESP_PH_WITH_ADC.h> 
#include <Adafruit_ADS1015.h>    
#include <DFRobot_ESP_EC.h>
#include <EEPROM.h>
#include "GravityTDS.h"
#include <WiFi.h>
#include <WebServer.h>
#include <LiquidCrystal_I2C.h>       // Include LiquidCrystal_I2C library
#include <OneWire.h>
#include <DallasTemperature.h>
// GPIO where the DS18B20 is connected to
const int oneWireBus = 4; 
    
#define TdsSensorPin 36 // Where Analog pin of TDS sensor is connected to arduino
 
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);
GravityTDS gravityTds;
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
float tdsValue = 0;
//..........................     
DFRobot_ESP_EC ec;
DFRobot_ESP_PH_WITH_ADC ph;
Adafruit_ADS1115 ads;
 
float phVoltage, phValue, phtemperature = 25;
float voltage, ecValue, temperature = 25;
int i,j,k;
//...................................
 
float temperature1, humidity, pressure, altitude;
String s_string; 
LiquidCrystal_I2C lcd(0x27, 20, 4);  // Configure LiquidCrystal_I2C library with 0x27 address, 20 columns and 4 rows

/* Put your SSID & Password */
const char* ssid = "IoT_Bioflok";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

/* Put IP Address details */
//IPAddress local_ip(192,168,1,11);
//IPAddress gateway(192,168,1,1);
//IPAddress subnet(255,255,255,0);

WebServer server(80);

bool air_pump;
bool fault= LOW;
bool fault1= LOW;
bool finger=LOW;

float TT;
float NH3;
int TDS;
byte LCD_clear=0;
float tempC,tempF;
//--------
byte buzzer = 5;

//==================== SETUP =====================
void setup() {
  Serial.begin(115200);
   sensors.begin();// DS18b20 temperature sensor
//....................
 EEPROM.begin(32);//needed EEPROM.begin to store calibration k in eeprom
  ph.begin();
  ec.begin();
   
//--------------
gravityTds.setPin(36);
gravityTds.setAref(5.0); //reference voltage on ADC, default 5.0V from NodeMCU
gravityTds.setAdcRange(1024); //1024 for 10bit ADC;4096 for 12bit ADC
gravityTds.begin(); //initialization
//----------------   
  lcd.init();  
  lcd.backlight();                   // Turn backlight ON
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to: ");
  lcd.setCursor(0, 1);
  lcd.println(ssid);
  lcd.setCursor(0, 2);
  lcd.print("Password: ");
  lcd.println(password);

  WiFi.softAP(ssid, password);
//  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(5100);
  
  server.begin();
  Serial.println("HTTP server started");
  Serial.println( WiFi.softAPIP() );  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("HTTP Server Started");
  lcd.setCursor(0, 2);
  lcd.print("IP:");
  lcd.println( WiFi.softAPIP() );  
  delay(2000);

  pinMode(buzzer, OUTPUT);
  pinMode(39, INPUT);
  pinMode(36, INPUT);
  pinMode(34, INPUT);

  
 digitalWrite(buzzer, HIGH);
  delay(500);
 digitalWrite(buzzer, LOW);
  
  LCD_demo();// to print project information
 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Please wait...10 sec.");
   
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
 }

 void(* resetFunc) (void) = 0; //declare reset function @ address 0

//======================= Main Loop ======================
void loop() {


  sensor_data();
  logic();
  display_data();
  server.handleClient();
}
//====================== END Main LOOP =======================
//-------------------
void handle_OnConnect() {

  temperature = TT;
  humidity = TDS;
  pressure = NH3;

  server.send(200, "text/html", SendHTML(temperature,humidity,pressure,altitude)); 
}
//----------------------------------------------------
void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}
//String SendHTML(uint8_t led1stat,uint8_t led2stat){
//------------------------------------------------------ 
String SendHTML(float temperature,float humidity,float pressure,float altitude){
  String ptr = "<!DOCTYPE html>";
  ptr +="<html>";
  ptr +="<head>";
  ptr +="<title>Iot based Biofloc monitoring</title>";
  ptr +="<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  ptr +="<meta http-equiv='refresh' content='5'/>\r\n";//--------------auto refresh page----------------------
  ptr +="<link href='https://fonts.googleapis.com/css?family=Open+Sans:300,400,600' rel='stylesheet'>";
  ptr +="<style>";
  ptr +="html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #444444;}";
  ptr +="body{margin: 0px;} ";
  ptr +="h1 {margin: 50px auto 30px;} ";
  ptr +=".side-by-side{display: table-cell;vertical-align: middle;position: relative;}";
  ptr +=".text{font-weight: 600;font-size: 19px;width: 200px;}";
  ptr +=".reading{font-weight: 300;font-size: 50px;padding-right: 25px;}";
  ptr +=".temperature .reading{color: #F29C1F;}";
  ptr +=".humidity .reading{color: #3B97D3;}";
  ptr +=".pressure .reading{color: #26B99A;}";
  ptr +=".altitude .reading{color: #955BA5;}";
  ptr +=".superscript{font-size: 17px;font-weight: 600;position: absolute;top: 10px;}";
  ptr +=".data{padding: 10px;}";
  ptr +=".container{display: table;margin: 0 auto;}";
  ptr +=".icon{width:65px}";
  ptr +="</style>";
  ptr +="</head>";
  ptr +="<body>";
  
  ptr +="<center><h1 style=color:#55cc99;>Iot based Biofloc monitoring</h1>";
  ptr +="<center><h2 style=color:#aa99ff;>Supervisor: Fahmida Sharmin Jui, Dept.of ECE</h2>";
  ptr +="<center><h3 style=color:#ff5511;>Md : Nazim Uddin  ID:008-15-87 ";
  ptr += "<br>"; // Go to the next line.
  ptr +="Md : Mizanur Rahman ID:008-15-89</h3>";
  ptr += "<br>"; // Go to the next line.
  
  if(digitalRead(buzzer)){ ptr += "<body style=background-color:#ffccff;>";}// for alarm color change to RED

  ptr +="<div class='container'>";
  ptr +="<center><div class='data temperature'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg enable-background='new 0 0 19.438 54.003'height=54.003px id=Layer_1 version=1.1 viewBox='0 0 19.438 54.003'width=19.438px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M11.976,8.82v-2h4.084V6.063C16.06,2.715,13.345,0,9.996,0H9.313C5.965,0,3.252,2.715,3.252,6.063v30.982";
  ptr +="C1.261,38.825,0,41.403,0,44.286c0,5.367,4.351,9.718,9.719,9.718c5.368,0,9.719-4.351,9.719-9.718";
  ptr +="c0-2.943-1.312-5.574-3.378-7.355V18.436h-3.914v-2h3.914v-2.808h-4.084v-2h4.084V8.82H11.976z M15.302,44.833";
  ptr +="c0,3.083-2.5,5.583-5.583,5.583s-5.583-2.5-5.583-5.583c0-2.279,1.368-4.236,3.326-5.104V24.257C7.462,23.01,8.472,22,9.719,22";
  ptr +="s2.257,1.01,2.257,2.257V39.73C13.934,40.597,15.302,42.554,15.302,44.833z'fill=#F29C21 /></g></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Temperature (ref:30;C)</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int)temperature;
  ptr +="<span class='superscript'>&deg;C</span></div>";
//  ptr +="</div>";
  
  ptr +="<center><div class='data humidity'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg enable-background='new 0 0 29.235 40.64'height=40.64px id=Layer_1 version=1.1 viewBox='0 0 29.235 40.64'width=29.235px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><path d='M14.618,0C14.618,0,0,17.95,0,26.022C0,34.096,6.544,40.64,14.618,40.64s14.617-6.544,14.617-14.617";
  ptr +="C29.235,17.95,14.618,0,14.618,0z M13.667,37.135c-5.604,0-10.162-4.56-10.162-10.162c0-0.787,0.638-1.426,1.426-1.426";
  ptr +="c0.787,0,1.425,0.639,1.425,1.426c0,4.031,3.28,7.312,7.311,7.312c0.787,0,1.425,0.638,1.425,1.425";
  ptr +="C15.093,36.497,14.455,37.135,13.667,37.135z'fill=#3C97D3 /></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>TDS(Ref:1400-1800)</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int)humidity;
  ptr +="<span class='superscript'>PPM</span></div>";
//  ptr +="</div>";
  
  ptr +="<center><div class='data pressure'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg enable-background='new 0 0 40.542 40.541'height=40.541px id=Layer_1 version=1.1 viewBox='0 0 40.542 40.541'width=40.542px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M34.313,20.271c0-0.552,0.447-1,1-1h5.178c-0.236-4.841-2.163-9.228-5.214-12.593l-3.425,3.424";
  ptr +="c-0.195,0.195-0.451,0.293-0.707,0.293s-0.512-0.098-0.707-0.293c-0.391-0.391-0.391-1.023,0-1.414l3.425-3.424";
  ptr +="c-3.375-3.059-7.776-4.987-12.634-5.215c0.015,0.067,0.041,0.13,0.041,0.202v4.687c0,0.552-0.447,1-1,1s-1-0.448-1-1V0.25";
  ptr +="c0-0.071,0.026-0.134,0.041-0.202C14.39,0.279,9.936,2.256,6.544,5.385l3.576,3.577c0.391,0.391,0.391,1.024,0,1.414";
  ptr +="c-0.195,0.195-0.451,0.293-0.707,0.293s-0.512-0.098-0.707-0.293L5.142,6.812c-2.98,3.348-4.858,7.682-5.092,12.459h4.804";
  ptr +="c0.552,0,1,0.448,1,1s-0.448,1-1,1H0.05c0.525,10.728,9.362,19.271,20.22,19.271c10.857,0,19.696-8.543,20.22-19.271h-5.178";
  ptr +="C34.76,21.271,34.313,20.823,34.313,20.271z M23.084,22.037c-0.559,1.561-2.274,2.372-3.833,1.814";
  ptr +="c-1.561-0.557-2.373-2.272-1.815-3.833c0.372-1.041,1.263-1.737,2.277-1.928L25.2,7.202L22.497,19.05";
  ptr +="C23.196,19.843,23.464,20.973,23.084,22.037z'fill=#26B999 /></g></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>NH3(Ref:0.01 mg/L)</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int)pressure;
  ptr +="<span class='superscript'>mg/L</span></div>";
  ptr +="</div><h1>";
  
//  if (Serial.available() > 0) {s_string=Serial.readString();}
//  ptr += s_temp;
//  ptr += s_bpm;
//  ptr += s_spo2;
if(air_pump)
 { ptr += "Air pump ON";}
 else{ptr += "Air pump OFF. Pls check";}
  
  ptr +="</div></h1>";
  ptr +="</body>";
  ptr +="</html>";
  return ptr;
}
//=============== LCD Demo =====================
void LCD_demo(){
    // Print a message to the LCD.
  lcd.setCursor(0, 0);
  lcd.print(" Iot based Biofloc  ");
  lcd.setCursor(0, 1);
  lcd.print("-----Supervisor-----");
  lcd.setCursor(0, 2);
  lcd.print("Fahmida Sharmin Jui ");
  lcd.setCursor(0, 3);
  lcd.print(" Department of ECE  ");
  delay(5000);  
  //IOT based Substation monitoring and controlling system
  lcd.setCursor(0, 0);
  lcd.print("Iot based Biofloc   ");
  lcd.setCursor(0, 1);
  lcd.print("Programing Team :   ");
  lcd.setCursor(0, 2);
  lcd.print(" Nazim     008-15-87");
  lcd.setCursor(0, 3);
  lcd.print(" Mizanur   008-15-89");
  
  delay(5000);  
  lcd.clear();
}

//=============== sensor Read ===================
void sensor_data(){
  delay(10);
  sensors.requestTemperatures(); 
  tempC = sensors.getTempCByIndex(0);
  tempF = sensors.getTempFByIndex(0);
  TT=tempC;

  if(i>=20){air_pump=0;i=0;}//air_pump=0;  
  i++;
  if(analogRead(34)<=400){air_pump=1;}// from sound sensor
  
  
//---------------
gravityTds.setTemperature(sensors.getTempCByIndex(0)); // grab the temperature from sensor and execute temperature compensation
gravityTds.update(); //calculation done here from gravity library
tdsValue = gravityTds.getTdsValue(); // then get the TDS value
 
 TDS=tdsValue;

 voltage=analogRead(36);
 //int val = analogRead(0);
// TDS = map(TDS, 0, 4095, 0, 100);
    ecValue = ec.readEC(voltage, tempC); // convert voltage to EC with temperature compensation

  ec.calibration(voltage, tempC); // calibration process by Serail CMD
  
//Simple relationships are used to convert EC to TDS, or vice Versa:
//  TDS (mg/L or ppm) = EC (dS/m) x 640 (EC from 0.1 to 5 dS/m)
//  TDS (mg/L or ppm) = EC (dS/m) x 800 (EC > 5 dS/m) 

if(ecValue<=5.0){TDS=ecValue*640;}
if(ecValue>=5.1){TDS=ecValue*800;}


//  phVoltage = ads.readADC_SingleEnded(1) / 10;
//      phValue = ph.readPH(phVoltage, tempC);
//      Serial.print("phValue: " );
//      Serial.println(phValue);
      
//TDS=phValue;
//---------------------------------- 
 NH3=analogRead(39);
 NH3 = map(NH3, 0, 4095, 0, 100);
//--------------------------  
 //display_data();
 //logic(); 
 }
//===================================
void logic(){

  if(!air_pump){digitalWrite(buzzer, !digitalRead(buzzer));}
  else{ digitalWrite(buzzer,LOW);}
  
}
//====================================
void display_data(){
 
//lcd.clear();
lcd.setCursor(0,0);
lcd.print(" IoT Based bioflok ");
lcd.setCursor(0,1);
lcd.print("Temp:");
lcd.print(TT);
    
    lcd.setCursor(0,2);
    lcd.print("EC:"); //
    lcd.print(ecValue,1); //

    lcd.print(", TDS:"); //
    lcd.print(TDS); //
    lcd.print(" PPM"); //

    //lcd.print(" ms/cm "); //
    
    lcd.setCursor(0,3);
    lcd.print("NH3:"); // 
    lcd.print(NH3); // 

//    lcd.print(" Airpump:");
if(air_pump){lcd.print(", Air ON"); }
else{lcd.print(" Air OFF");}
}
