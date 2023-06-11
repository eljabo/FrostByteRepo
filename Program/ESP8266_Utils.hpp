String ApiHost = "http://192.168.115.134:5000";
bool correr = true;

void ConnectWiFi_AP() {
  Serial.println();
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP8266", "11221122");
  Serial.println(WiFi.softAPIP());
  correr = false;
}


void ConnectWiFi_STA() {
  WiFi.mode(WIFI_STA);
  Serial.println("ssid: " + String(ssid) + " contraseña: " + String(password));
  WiFi.begin(ssid, password);

  unsigned long startTime = millis();
  int timeout = 30000; // Tiempo en milisegundos para salir del bucle (30 segundos)

  while (WiFi.status() != WL_CONNECTED && millis() - startTime < timeout) {
    delay(100);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Conexión WiFi establecida");

    IPAddress gateway = WiFi.gatewayIP();
    IPAddress subnetMask = WiFi.subnetMask();

    // Obtener el último byte de la dirección IP del gateway
    uint8_t lastByte = gateway[3];
    lastByte += 10; // Incrementar en 10 para obtener la nueva dirección IP

    // Configurar una dirección IP estática
    IPAddress staticIP(gateway[0], gateway[1], gateway[2], lastByte);
    WiFi.config(staticIP, gateway, subnetMask);

    Serial.print("Dirección IP asignada manualmente: ");
    Serial.println(staticIP);
    Serial.print("Dirección IP asignada: ");
    Serial.println(WiFi.localIP());
    ApiHost = "http://" + String(gateway[0]) + "." + String(gateway[1]) + "." + String(gateway[2]) + ".69:5000";
    Serial.print("Dirección IP esperada: ");
    Serial.println(ApiHost);
  } else {
    Serial.println("No se pudo conectar al WiFi");
  }
}
//Esta función lee el servidor activo del Json y lo pone como servidor actual, antes de conectarse a internet.
//en resumen se conecta al servidor anterior
void leerserver(){
  DynamicJsonDocument jsonDoc(200);
  File readFile = SPIFFS.open("/server.json", "r");
  if (!readFile) {
    Serial.println("No se pudo abrir el archivo de configuración");
  }
  //Se vuelca el contenido en una variable
     String fileContent = readFile.readString();
  //Se deserializa
    DeserializationError error = deserializeJson(jsonDoc, fileContent);
    if (error) {
      Serial.println("No se pudo parsear el JSON");
      return;
    }
//lee el Json entero en un bucle for
    uint8_t numRecords = jsonDoc.size();

    String situacion;

    for (uint8_t i = 0; i < numRecords; i++) {
      // Obtener el valor del campo "servidores" del registro actual
      String servstr = jsonDoc[i]["servidores"].as<String>();
      // Obtener el valor del campo "estado" del registro actual
      String ststr = jsonDoc[i]["estado"].as<String>();
      if (ststr == "ACTIVO") {
        //si hay servidor activo se actualiza la IP válida
        strcpy(validip, servstr.c_str());
        //También se actualiza la dirección IP de la API
        ApiHost = "http://" + String(validip) + ":5000";
        Serial.println("Ip valida añadida: " + String(validip));
      }

    }

  readFile.close();

}
