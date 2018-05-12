// programa base
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFiClient.h>


#define D0 16
#define D2 4
//#define D3 0
#define D4 2
#define D5 14
//#define D6 12
//#define D7 13
//#define D8 15
//#define D9 3

bool flaginterr = false;
bool flagauxilio = false;
const int interruptPin = 5;
int8_t ret;
void MQTT_connect();
const char* nameOta = "espmaster";

//char __ssssid[] = "RODRIGO_ROMAN"; //  your network SSID (name)
//char __spasswd[] = "rodrigo1964";    // your network password (use for WPA, or use as key for WEP)

//char __ssssid[] = "dlink 1"; //  your network SSID (name)
//char __spasswd[] = "liguista123";    // your network password (use for WPA, or use as key for WEP)

//char __ssssid[] = "VILLACIS"; //  your network SSID (name)
//char __spasswd[] = "Ambato2019";    // your network password (use for WPA, or use as key for WEP)

char __ssssid[] = "IOT-IEEE"; //  your network SSID (name)
char __spasswd[] = "IOTIEEEUNLP";    // your network password (use for WPA, or use as key for WEP)

WiFiClient client;
//Adafruit_MQTT_Client mqtt(&client,"159.203.139.127", 1883);
Adafruit_MQTT_Client mqtt(&client,"192.168.0.90", 1883);


Adafruit_MQTT_Publish focoestado = Adafruit_MQTT_Publish(&mqtt,"home/master/switch1");
Adafruit_MQTT_Publish auxestado = Adafruit_MQTT_Publish(&mqtt,"home/master/switch1/aux");
Adafruit_MQTT_Publish inicio = Adafruit_MQTT_Publish(&mqtt,"home/master/switch1/inicio");
// Setup a feed called 'time' for subscribing to current time
Adafruit_MQTT_Subscribe foco = Adafruit_MQTT_Subscribe(&mqtt, "home/master/switch1/set");
Adafruit_MQTT_Subscribe auxilio = Adafruit_MQTT_Subscribe(&mqtt, "home/master/switch1/set/aux");


void fococallback(char *x, uint16_t len) {
  noInterrupts();
  Serial.println(x);
  //Serial.println(flaginterr);
  
  String mijin=String(x);
  if(mijin=="ON" && flagauxilio == false){
    digitalWrite(D0,HIGH);
    focoestado.publish("ON");
    //Serial.println("envio ON"); 
    
  }else if(mijin=="OFF" && flagauxilio == false){
    digitalWrite(D0,LOW);
    //Serial.println("envio OFF");
    focoestado.publish("OFF");
  }
 interrupts();
}


void auxcallback(char *x, uint16_t len) {
  noInterrupts();
  Serial.println(x);
  //Serial.println(flaginterr);
  
  String mijin=String(x);
  if(mijin=="ON"){
    flagauxilio = true;
    digitalWrite(D2,LOW);
    auxestado.publish("Modo manual unicamente");
  }
  if(mijin=="OFF"){
    digitalWrite(D2,HIGH);
    flagauxilio = false;
    auxestado.publish("Modo manual desactivado");
  }
  interrupts();
}

void handleInterrupt() { 
 // accion momento de una interrupcion
  //Serial.println("Interrupt");
  if (flaginterr == false){
    flaginterr = true;  
  }
  
}

void setup() {
  //noInterrupts();
  pinMode(D2,OUTPUT);
  digitalWrite(D2,LOW);
  pinMode(D0,OUTPUT);
  digitalWrite(D0,LOW);
  EEPROM.begin(400);
  Serial.begin(115200);
  pinMode(interruptPin,INPUT); 
  delay(500);
  pinMode(10,OUTPUT);
  pinMode(D4,OUTPUT);
  pinMode(D5,OUTPUT);
  digitalWrite(D4,LOW);
  digitalWrite(D5,LOW);
  //attachInterrupt(interruptPin, handleInterrupt, RISING); 
  //attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, CHANGE); 
  delay(50);
  if (flaginterr == true)
    {
      Accion();
    }
  Serial.println(F("Iniciando...."));
  //Conectando a red WiFI
  wifi_conection();
  OTA_set();
  //keepalive.setCallback(keepalive1);
  foco.setCallback(fococallback);
  auxilio.setCallback(auxcallback);
  //Setup MQTT subscription for time feed.
  mqtt.subscribe(&foco);
  mqtt.subscribe(&auxilio);
  MQTT_connect(); // una vez conectado a la red WiFi, busca coneccion al servidor MQTT
  delay(200);
  inicio.publish(1);
  if (digitalRead(interruptPin) == HIGH){
    digitalWrite(D0,HIGH);
  }else if (digitalRead(interruptPin) == LOW){
    digitalWrite(D0,LOW);
  }
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, CHANGE); 
  digitalWrite(D2,LOW);
  //interrupts();
  //FIN SETUP
}


void OTA_set(){
  ArduinoOTA.setHostname(nameOta);
  // No authentication by default
  ArduinoOTA.setPassword((const char *)"yotec");
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready OTA");
}

void wifi_conection(){
  
  Serial.println("Conectando a una red WiFi");
  //WiFi.mode(WIFI_STA);
  //WiFi.config(ip, gateway, subnet);
  WiFi.begin(__ssssid,__spasswd);
  
  while ((WiFi.status() != WL_CONNECTED))// && retries <= 70) 
  {
    digitalWrite(10,LOW);
    //digitalWrite(D2,LOW);
    Serial.print("*");
    WiFi.begin(__ssssid,__spasswd);
    delay(250);
    Serial.print(".");
    if (flaginterr == true)
       {
         Accion();
       }
  }
  
  if (WiFi.status() == WL_CONNECTED) 
  {
    Serial.println(F("WiFi conectado exitosamente"));
    digitalWrite(10,HIGH);
  }
  
  //Serial.println(F("Conectado")); 
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
 
//FIN WIFI
}



void loop() {
  ArduinoOTA.handle();
// Valida estar conectado a una red WiFI
  mqtt.processPackets(900);
  if (WiFi.status() != WL_CONNECTED)
    { 
      digitalWrite(10,LOW);
      Serial.println("Conectando a red WiFi");
      wifi_conection();
      if(digitalRead(D0) == HIGH){
        focoestado.publish("ON");
      }else if(digitalRead(D0) == LOW){
        focoestado.publish("OFF");
      }
    } 
// Valida estar conectado al servidor MQTT  
  if ((ret = mqtt.connect()) != 0) 
  {
    digitalWrite(D5,LOW);
    Serial.println("Conectando a servidor MQTT");
    MQTT_connect();
    if(digitalRead(D0) == HIGH){
       focoestado.publish("ON");
    }else if(digitalRead(D0) == LOW){
       focoestado.publish("OFF");
    }
  }

// Cambio de estado de switch segun flag de interrupcion
  if (flaginterr == true)
  {
   Accion();
  }

  while(flagauxilio == true){
    digitalWrite(D2,LOW);
    Serial.println("manual");
    ArduinoOTA.handle();
    mqtt.processPackets(900);
    delay(1000);
  }

//FIN LOOP
}



void Accion(){
  noInterrupts();
   if (digitalRead(D0) == HIGH){
     //Serial.println("interrupcion rele cambia a abierto");  
     //fococ=false;
     digitalWrite(D0,LOW);
     focoestado.publish("OFF");
     //Serial.println("enviado OFF");
     
    }else if (digitalRead(D0) == LOW){
      //Serial.println("interrupcion rele cambia a cerrado");  
      //fococ=true;
      digitalWrite(D0,HIGH);
      focoestado.publish("ON");
      //Serial.println("enviado ON");
    }
    Serial.println("accion realizada");
    delay(700);
    flaginterr = false;
    interrupts();
  
}

void MQTT_connect() {
  // Coneccion a servidor MQTT
  // Stop if already connected.
  if (mqtt.connected()) {
    Serial.println("Conectado a servidor");
    //digitalWrite(D5,HIGH);
    return;
  }

  Serial.print("Conectando a  MQTT... ");
  int retries=20;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       digitalWrite(D4,LOW);
       //Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Reintentado Conexion...");
       mqtt.disconnect();
       if (flaginterr == true)
        {
          Accion();
        }
       //delay(200);  // wait 5 seconds
       retries--;
       if (retries == 0) {
          Serial.println("Validando conexion a WiFi");
          //delay(200);
          if (WiFi.status() != WL_CONNECTED)
            { 
              Serial.println("Reconectando a red WiFi");
              wifi_conection();
              
            } else {
              Serial.println("Conexion a wifi estable");
            }
          retries =20;
      }
  }
  Serial.println("Conectado al servidor MQTT!");
  
}
