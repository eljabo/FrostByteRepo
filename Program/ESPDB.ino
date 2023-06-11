//ROL abrevianción de RELÉ OR LED
const uint8_t ROL = D8;

static char ssid[25] = "OPPO Reno5 Z";
static char password[25] = "12345678";
//Formenviado se usa para comprobar si el formulario se ha enviado
boolean formEnviado = true;

char validip[18] = "192.168.99.135";

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <EEPROM.h>
#include <pgmspace.h>
#include <regex>

//Variables de las horas y minuto de inicio y de fin de la mañana y la tarde
static uint8_t horamananainicio = 7;
static uint8_t minutomananainicio = 30;
static uint8_t horamananafin = 9;
static uint8_t minutomananafin = 00;
static uint8_t horatardeinicio = 8;
static uint8_t minutotardeinicio = 16;
static uint8_t horatardefin = 14;
static uint8_t minutotardefin = 30;

static uint8_t thoras = 0;
static uint8_t tminutos = 0;

//Esta función da formato a los números de hora y minuto haciéndo un String con este formato 09:00
  static String formateado(int hora, int minuto) {
  String horaFormateada = hora < 10 ? "0" + String(hora) : String(hora);
  String minutoFormateado = minuto < 10 ? "0" + String(minuto) : String(minuto);
  return horaFormateada + ":" + minutoFormateado;
}

//Esta función comprueba, las horas y los minutos para saber si el LED debe encenderse o no
bool comparador(uint8_t horainicial, uint8_t minutoinicial, uint8_t horafinal, uint8_t minutofinal) {
  if (thoras > horainicial || (thoras == horainicial && tminutos >= minutoinicial)) {
    if (thoras < horafinal || (thoras == horafinal && tminutos <= minutofinal)) {
      digitalWrite(ROL, HIGH);
      Serial.println("Encendido");
      return true;
    }
  }
  digitalWrite(ROL, LOW);
  Serial.println("Apagado");
  return false;
}

bool led_horarios() {
  bool comparadormanana = comparador(horamananainicio, minutomananainicio, horamananafin, minutomananafin);
  bool comparadortarde = comparador(horatardeinicio, minutotardeinicio, horatardefin, minutotardefin);

  if (comparadormanana || comparadortarde) {
    return true;
  } else {
    return false;
  }
}

//Se declaran variables globales, que más tarde se concatenaran con otras
//Los curiosos valores actuales sirven para debugear, ya que en caso de fallar las funciones que concaterán su futuro valor o cualquier otro aspecto
//se mostrará este valor existen otros valores, para indicar que los futuros valores se están cargando en la página de la misma forma que estos indican error
  
  static char horariomanana[50];
  static char horariotarde[50];
  
//Declaraciones necesarias del sensor de temperatura
const int DHTPIN = D4;
    #define DHTTYPE DHT11  
    DHT dht(DHTPIN, DHTTYPE);
    uint8_t humidity = 0.0;
    uint8_t temperature = 0.0;   
    char hora[15];
    char nuevahora[15];
    static WiFiUDP ntpUDP;
    static NTPClient timeClient(ntpUDP, "0.europe.pool.ntp.org", 7200, 6000);
 //Es la variable de la temperatura de corte
static uint8_t cutemp = 15;
//Se inicia el cliente http de los archivos de configuración en otras circunstancias estaría en SETUP, pero la situación lo requiere así

#include "ESP8266_Utils.hpp"
#include "API.hpp"
#include "ServerDB.hpp"


//Se declarará otra variable en el futuro ya que se usará otro Json para los servidores activos
 
void setup() 
{
Serial.begin(115200);
  //Se inicializan las librerías
   dht.begin();
     SPIFFS.begin();
      EEPROM.begin(256);

    leerserver();
   
    //Se lee la memoria EEPROM       
    epromX(false);

    pinMode(ROL, OUTPUT);
    
    //Se intenta conectar al wifi con los datos actuales, almacenados de la última vez que se modificó el formulario del modo AP
    ConnectWiFi_STA();
     if (WiFi.status() == WL_CONNECTED) {
      InitServer();
}
              else{
                    ConnectWiFi_AP();
                    initAP();
                      }
                   
   
     timeClient.begin();
        timeClient.update();

//Se actualiza el Json de los horarios bajándolos de la raspi
      actualizar();
//Lo mismo pero con el Json de los servidores disponibles
      actualizarServer();
//Se actualiza el servidor css usado según el servidor que esté disponible en todos los archivos esto es muy exigente.

          cambiarDireccionesHTML("/index.html");
          cambiarDireccionesHTML("/Servidores.html");
          cambiarDireccionesHTML("/Hora.html");
          cambiarDireccionesHTML("/Temperatura.html");

}
  
void loop(){
  //Estas variables contienen la hora y el minuto en el que estamos ahora mismo
  thoras = timeClient.getHours();    
  tminutos = timeClient.getMinutes();
  

    //Se usa esta función para actualizar la temperatura la hora y demás cosas por interválos, para evitar la sobrecarga del ESP
    horatemp();
  
    
   //Cuando el formulario pasa a ser true se sube el Json actualizado a la base de datos
   if (formEnviado) {
    StaticJsonDocument<200> jsonDoc;
    subir(jsonDoc, "/datos.json");
   }
   
//Esta variable se torna en true cuando se envía el formulario, por tanto después hay apagarla otra vez
 formEnviado = false;    
 

//Si se pierde la conexión el ESP se reiniciará y se levantará en modo AP si no es capaz de volver a conectarse
 if (correr && WiFi.status() != WL_CONNECTED) {
  
  ESP.restart();
  
}

//datos nuevos comprueba si se ha enviado el formulario AP con nuevos datos reinicia el ESP
//La función que lo torna a true graba los datos en la eeprom al final, por tanto es seguro reiniciar
if (datosnuevos) {
   ESP.restart();
  }

// Obtener el tamaño de la memoria libre
uint32_t freeHeap = ESP.getFreeHeap();

// Imprimir el resultado
Serial.print("cantidad de memoria desocupada: ");
Serial.print(freeHeap);
Serial.println("%");
Serial.println(leer("/server.json"));
Serial.println(String(validip));
delay(1000);
}
