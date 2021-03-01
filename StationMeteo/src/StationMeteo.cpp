#include "StationMeteo.h"
#include <Credential.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>
#include <WifiManager.h>
#include <Arduino.h>

StationMeteo::StationMeteo(){};

// wifimanager
WiFiManager wm;
bool initialConfig =false;

LiquidCrystal_I2C lcd(0x27, 16, 2);
const char* ssid = MYSSID;
const char* password =  MYPSW;
char hostname[] ="192.168.43.172";
const int mqttPort = 1883;
const char* mqttUser = "mqttUser";
const char* mqttPassword = "Tebouza25";

//sortie dels et buzzer
const int pinDelRouge = 12;
const int pinDelJaune = 13;
const int pinDelBleu = 14;
const int pinBuzzer = 27;
//RVB
const int borneRvbRouge = 5;
const int borneRvbVerte = 23;
const int borneRvbBlue = 19;

// Temperature Humidite Pression MQTT Topics
char MqttPubTemp[] = "esp32/bme280/temperature";
char MqttPubHum[] = "esp32/bme280/humidite";
char MqttPubPress[] = "esp32/bme280/pression";
float temperature;
float humidite;
float pressionAtmospherique;
Adafruit_BME280 bme;
WiFiClient wifiClient;
PubSubClient client(wifiClient);
int status = WL_IDLE_STATUS;


void StationMeteo::reset()
{
    // initialiser LCD screen
    lcd.init();
    lcd.backlight();
    Serial.begin(115200);
    WiFi.begin(ssid, password);
       
    pinMode(pinDelRouge, OUTPUT);
    pinMode(pinDelJaune, OUTPUT);
    pinMode(pinDelBleu, OUTPUT);
    pinMode(pinBuzzer, OUTPUT);
    pinMode(borneRvbRouge, OUTPUT);
    pinMode(borneRvbVerte, OUTPUT);
    pinMode(borneRvbBlue, OUTPUT);

    // default settings
    // (you can also pass in a Wire library object like &Wire2)
    bool status;
    status = bme.begin(0x76);  
    if (!status) {
        Serial.println("connexion échoue avec le sensor BME280, verfier les cables!");
        while (1);
    }
    //wifimanager
   if(WiFi.SSID()== ""){
      initialConfig = true;
    }
    else{
      WiFi.mode(WIFI_STA);
      int connRes = WiFi.waitForConnectResult();
      Serial.println(connRes);
    }
    if(WiFi.status() != WL_CONNECTED){
         Serial.println("non connecte") ;
    }
    else{
      Serial.print("Local ip:") ;
      Serial.println(WiFi.localIP()) ;
    }
    //parametre mqtt dans le portail 
    wm.setConfigPortalTimeout(300);
    WiFiManagerParameter custom_Text("<p> Remplir les cases suivant </p>");
    wm.addParameter(&custom_Text);
    char mqtt_server[20] = "192.168.43.172";
    WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
    wm.addParameter(&custom_mqtt_server);
    char mqtt_port[20] = "1883";
    WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 40);
    wm.addParameter(&custom_mqtt_port);
    char mqtt_user[20] = "mqttUser";
    WiFiManagerParameter custom_mqtt_user("user", "mqtt user", mqtt_user, 40);
    wm.addParameter(&custom_mqtt_user);
    char mqtt_password[20] = "mqttPassword";
    WiFiManagerParameter custom_mqtt_password("password", "mqtt password", mqtt_password, 40,"type=\"password\"");
    wm.addParameter(&custom_mqtt_password);
    wm.startConfigPortal(ssid,password);
    client.setServer(hostname, 1883); //default port for mqtt is 1883
    // default settings
}

void StationMeteo::afficherLCD()
{
  lcd.setCursor(0, 0);
  lcd.print("Temperature : ");
  lcd.setCursor(0, 1);
  lcd.print(temperature);
  lcd.print(" degre C");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("humidite : ");
  lcd.setCursor(0, 1);
  lcd.print(humidite);
  lcd.print(" %");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("pression : ");
  lcd.setCursor(0, 1);  
  lcd.print(pressionAtmospherique);
  lcd.print(" Pa");
  delay(2000);
  lcd.clear();
}



void StationMeteo::afficherDelHumidite()
{
    if(humidite > 40){
      afficherRvb(0xff0000L);  
    } 
    else{
       afficherRvb(0x008000L);
    }         
}
void StationMeteo::afficherRvb(long p_Coleur)
{
  int vRouge = (p_Coleur >> 16) & 0xFF;
  int vVert = (p_Coleur >> 8) & 0xFF;
  int vBleu = p_Coleur & 0xFF;
  digitalWrite(borneRvbRouge, vRouge);
  digitalWrite(borneRvbVerte, vVert);
  digitalWrite(borneRvbBlue, vBleu);
}
void StationMeteo::afficherDels()
{
    if(temperature > 28){
       digitalWrite(pinDelRouge,HIGH);
       digitalWrite(pinBuzzer,HIGH);  
       digitalWrite(pinDelJaune,LOW);
       digitalWrite(pinDelBleu,LOW);

    }
    else if (temperature > 25)
    {
       digitalWrite(pinDelRouge,LOW);
       digitalWrite(pinBuzzer,LOW);  
       digitalWrite(pinDelJaune,HIGH);
       digitalWrite(pinDelBleu,LOW);
    }
    else{
       digitalWrite(pinDelRouge,LOW);
       digitalWrite(pinBuzzer,LOW);  
       digitalWrite(pinDelJaune,LOW);
       digitalWrite(pinDelBleu,HIGH);
    }    
}
   
void StationMeteo::executerMqtt()
{
    humidite= bme.readHumidity();
    temperature = bme.readTemperature(); 
    pressionAtmospherique = bme.readPressure()/100.0F ;
    
    char tempString[8];
    dtostrf(temperature, 1, 2, tempString);
    char humString[8];
    dtostrf(humidite, 1, 2, humString);
    char pressString[8];
    dtostrf(pressionAtmospherique, 1, 2, pressString);

    client.publish(MqttPubTemp,tempString); 
    client.publish(MqttPubHum,humString); 
    client.publish(MqttPubPress,pressString); 
    Serial.print("température : ");
    Serial.println(tempString);
    Serial.print("humidite : ");
    Serial.println(humString);
    Serial.print("pression : ");
    Serial.println(pressString);

}

void StationMeteo::reconnect()
{
    while (!client.connected()) {
        status = WiFi.status();
        if ( status != WL_CONNECTED) {
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        }
        Serial.println("Connection etablie");
        }
        Serial.print("Connecter avec home assistance …");
        Serial.print("192.168.43.172");
        if ( client.connect("Esp32Client",mqttUser,mqttPassword) )
        {
        Serial.println("[DONE]" );
        }
        else {
        Serial.println( " : retrying in 5 seconds]" );
        delay( 5000 );
        }
    }   
}