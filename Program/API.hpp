
AsyncWebServer server(80);

void epromX(bool writeMode) {
  if (writeMode) {
    // Escribir los valores en la EEPROM
    EEPROM.put(0, horamananainicio);
    EEPROM.put(sizeof(uint8_t), minutomananainicio);
    EEPROM.put(sizeof(uint8_t) * 2, horamananafin);
    EEPROM.put(sizeof(uint8_t) * 3, minutomananafin);
    EEPROM.put(sizeof(uint8_t) * 4, horatardeinicio);
    EEPROM.put(sizeof(uint8_t) * 5, minutotardeinicio);
    EEPROM.put(sizeof(uint8_t) * 6, horatardefin);
    EEPROM.put(sizeof(uint8_t) * 7, minutotardefin);
    EEPROM.put(sizeof(uint8_t) * 8, cutemp);

    // Escribir ssid en la EEPROM
    uint8_t ssidLength = sizeof(ssid);
    for (uint8_t i = 0; i < ssidLength; i++) {
      EEPROM.write(sizeof(uint8_t) * 9 + i, ssid[i]);
    }

    // Escribir password en la EEPROM
    uint8_t passwordLength = sizeof(password);
    for (uint8_t i = 0; i < passwordLength; i++) {
      EEPROM.write(sizeof(uint8_t) * 9 + ssidLength + i, password[i]);
    }

        uint8_t ipLength = sizeof(validip);
    for (uint8_t i = 0; i < ipLength; i++) {
      EEPROM.write(sizeof(uint8_t) * 9 + ssidLength + i, password[i]);
    }

    EEPROM.commit();
  } else {
    // Leer los valores desde la EEPROM
    horamananainicio = EEPROM.read(0);
    minutomananainicio = EEPROM.read(sizeof(uint8_t));
    horamananafin = EEPROM.read(sizeof(uint8_t) * 2);
    minutomananafin = EEPROM.read(sizeof(uint8_t) * 3);
    horatardeinicio = EEPROM.read(sizeof(uint8_t) * 4);
    minutotardeinicio = EEPROM.read(sizeof(uint8_t) * 5);
    horatardefin = EEPROM.read(sizeof(uint8_t) * 6);
    minutotardefin = EEPROM.read(sizeof(uint8_t) * 7);
    cutemp = EEPROM.read(sizeof(uint8_t) * 8);

    // Leer ssid desde la EEPROM
    uint8_t ssidLength = sizeof(ssid);
    for (uint8_t i = 0; i < ssidLength; i++) {
      ssid[i] = EEPROM.read(sizeof(uint8_t) * 9 + i);
    }

    // Leer password desde la EEPROM
    uint8_t passwordLength = sizeof(password);
    for (uint8_t i = 0; i < passwordLength; i++) {
      password[i] = EEPROM.read(sizeof(uint8_t) * 9 + ssidLength + i);
    }

       uint8_t ipLength = sizeof(validip);
    for (uint8_t i = 0; i < ipLength; i++) {
      password[i] = EEPROM.read(sizeof(uint8_t) * 9 + ssidLength + i);
    }
  }
}




//Esta función es auténtica magia y me costó una barbaridad comprenderla ASCI 48-57
void extractTime(const char* timeStr, uint8_t& hours, uint8_t& minutes) {
  //Esta variable coge el primer dígito de la cadena de texto le resta el valor ASCI osea si es 2 sería 50 y se le resta el valor 0 de ASCII = 48 - 50 = 2
  // tras obtener el 2 lo multiplicamos por 10 y le sumamos el siguiente dígito de la misma cadena y le restamos el 0 de ASCII que si es un 3 seria = 48 - 51 = 3
  //Estp daría lugar a la siguiente operación 2*10 = 20 + 3 = 23 esta sería la forma de conseguir el número de una cadena
  hours = (timeStr[0] - '0') * 10 + (timeStr[1] - '0');
  //Es el mismo procedimiento pero usando los dígitos correspondientes a los minutos
  minutes = (timeStr[3] - '0') * 10 + (timeStr[4] - '0');
}
//Solo guarda los valores de cada minuto y hora en cada una de sus variables y lo guarda en el EEPROM
void getHorarios(JsonDocument& jsonDoc, uint8_t index, uint8_t& horamananainicio, uint8_t& minutomananainicio, uint8_t& horamananafin, uint8_t& minutomananafin, uint8_t& horatardeinicio, uint8_t& minutotardeinicio, uint8_t& horatardefin, uint8_t& minutotardefin) {
  // Obtener las cadenas de tiempo desde el JSON
  static const char* m_inicio_str = jsonDoc[index]["m_inicio"];
  static const char* m_fin_str = jsonDoc[index]["m_fin"];
  static const char* t_inicio_str = jsonDoc[index]["t_inicio"];
  static const char* t_fin_str = jsonDoc[index]["t_fin"];

  // Extraer los valores de horas y minutos
  extractTime(m_inicio_str, horamananainicio, minutomananainicio);
  extractTime(m_fin_str, horamananafin, minutomananafin);
  extractTime(t_inicio_str, horatardeinicio, minutotardeinicio);
  extractTime(t_fin_str, horatardefin, minutotardefin);

  epromX(true);
}

void subir(JsonDocument& jsonDocument, const char* filename) {
  // Abrir el archivo JSON para lectura
  File readFile = SPIFFS.open(filename, "r");
  if (!readFile) {
    Serial.println("No se pudo abrir el archivo de configuración");
    return;
  }
  
  // Leer el contenido del archivo y cerrarlo
  String fileContent = readFile.readString();
  readFile.close();

  // Deserializar el archivo JSON en un objeto StaticJsonDocument
  //No te creas que sé muy bien lo que es deserializar
  deserializeJson(jsonDocument, fileContent);
  
  // Crear un objeto HTTPClient y establecer la URL del servidor de la base de datos
  //No me siento muy seguro al respecto y lo vuelvo a crear
  WiFiClient client;
  HTTPClient http;
  //Este es el endpoint del API REST de la raspi, ahí es dónde envíamos nuestro Json
  http.begin(client, ApiHost + "/api/actualizar");

  // Agregar la cabecera "Content-Type: application/json"
  http.addHeader("Content-Type", "application/json");

  // Serializar el objeto StaticJsonDocument en un JSON
  String jsonString;
  serializeJson(jsonDocument, jsonString);

  // Enviar la solicitud POST al servidor
  int httpResponseCode = http.POST(jsonString);
  http.end();
  

  // Verificar si la solicitud se ha enviado correctamente
  if (httpResponseCode == HTTP_CODE_OK) {
    Serial.println("Archivo JSON enviado con éxito a la base de datos");
    Serial.println(fileContent);
  } else {
    Serial.print("Error al enviar el archivo JSON a la base de datos. Código de respuesta: ");
    Serial.println(httpResponseCode);
    Serial.print("Descripción del error: ");
    Serial.println(http.errorToString(httpResponseCode).c_str());
   
  }


}

//Esta función se usa para guardar el archivo, hay que declararlo previamente
void save(JsonDocument& jsonDoc, const char* filename) {
  // Abrir el archivo JSON en modo escritura
  File configFile = SPIFFS.open(filename, "w");
  if (!configFile) {
    Serial.println("No se pudo abrir el archivo de configuración");
  } 
  // Escribir el objeto JSON en el archivo
  bool success = serializeJson(jsonDoc, configFile);
  configFile.flush(); 
  configFile.close();

  // Verificar si la operación de escritura fue exitosa
  if (success) {
    Serial.println("Archivo JSON creado correctamente");
    formEnviado = true;
  } else {
    Serial.println("Error al crear el archivo JSON");
  }
}

//Esta función lee un archivo Json, el que tu quieras
String leer(const char* filename) {
  File readFile = SPIFFS.open(filename, "r");
  if (!readFile) {
    Serial.println("No se pudo abrir el archivo de configuración");
  }

  // Leer el contenido del archivo y cerrarlo
  String fileContent = readFile.readString();
  readFile.close();

  // Devolver el contenido del archivo como una cadena
  return fileContent;

}

//Esta función recibe el Json y lo escribe de la base de datos
void actualizar() {
  // Se vuelve a crear el cliente http
  WiFiClient client;
  HTTPClient http;

  // Enviamos una petición a la API
  http.begin(client, ApiHost + "/api/mor");
  int httpCode = http.GET();
  http.addHeader("Content-Type", "application/json");

  // Se revisa el código de respuesta
  if (httpCode == HTTP_CODE_OK) {
    // Se parsea la respuesta
    StaticJsonDocument<200> jsonDoc;
    String payload = http.getString();
    DeserializationError error = deserializeJson(jsonDoc, payload);
    if (error) {
      Serial.println("No hubo parseito, ni Json, bonito");
      return;
    }
    //Aquí se escriben todos los campos Del Json, se usa la función get horarios para dar valores nuevos a las variables de horas y minutos
        String inicio = jsonDoc[0]["m_inicio"];
        Serial.println(inicio);
        String fin = jsonDoc[0]["m_fin"];
        Serial.println(fin);
        String tinicio = jsonDoc[0]["t_inicio"];
        Serial.println(tinicio);
        String tfin = jsonDoc[0]["t_fin"];
        Serial.println(tfin);
        String cortemp = jsonDoc[0]["cutemp"];
        
              jsonDoc["m_inicio"] = inicio;
              jsonDoc["m_fin"] = fin;
              jsonDoc["m_tinicio"] = tinicio;
              jsonDoc["m_tfin"] = tfin;
              jsonDoc["cutemp"] = cortemp;

        getHorarios(jsonDoc, 0, horamananainicio, minutomananainicio, horamananafin, minutomananafin, horatardeinicio, minutotardeinicio, horatardefin, minutotardefin);
   //Se guarda el Json nuevo y se lee
         save(jsonDoc, "/datos.json");
         Serial.println(leer("/datos.json"));
         epromX(false);

    Serial.println("Archivo actualizado ;)");
  } else {
    Serial.print("La raspi UNO no quiere a ningUNO");
    Serial.println(httpCode);
  }

  http.end();
}

void actualizarServer() {
  // Se vuelve a crear el cliente http
  WiFiClient client;
  HTTPClient http;

  // Enviamos una petición a la API
  http.begin(client, ApiHost + "/api/server");
  http.addHeader("Content-Type", "application/json");
  uint8_t httpCode = http.GET();

  // Se revisa el código de respuesta
  if (httpCode == HTTP_CODE_OK) {
    // Se parsea la respuesta
    StaticJsonDocument<200> jsonDoc;
    String payload = http.getString();
    DeserializationError error = deserializeJson(jsonDoc, payload);
    if (error) {
      Serial.println("No se pudo parsear el JSON");
      return;
    }

    // Leer y actualizar los campos del JSON
    String servidores = jsonDoc[0]["servidores"].as<String>();
    String estado = jsonDoc[0]["estado"].as<String>();

    jsonDoc[0]["servidores"] = servidores;
    jsonDoc[0]["estado"] = estado;

    uint8_t numRecords = jsonDoc.size();

    String situacion;

    for (uint8_t i = 0; i < numRecords; i++) {
      // Obtener el valor del campo "servidores" del registro actual
      String servstr = jsonDoc[i]["servidores"].as<String>();
      // Obtener el valor del campo "estado" del registro actual
      String ststr = jsonDoc[i]["estado"].as<String>();
      if (ststr == "ACTIVO") {
        strcpy(validip, servstr.c_str());
        ApiHost = "http://" + String(validip) + ":5000";
        Serial.println("Ip valida añadida: " + String(validip));
      }
      situacion += " Servidor: " + servstr + " Estado: " + ststr + "\n";
    }

    server.on("/state", HTTP_GET, [situacion](AsyncWebServerRequest *request) {
      request->send(200, F("text/html"), situacion);
    });

    // Guardar y leer el JSON actualizado
    save(jsonDoc, "/server.json");
    String ver = leer("/server.json");
    Serial.println(ver);
    epromX(false);

    Serial.println("Archivo actualizado ;)");
  } else {
    Serial.print("La rasp UNO no responde. Código de respuesta: ");
    Serial.println(httpCode);
  }
  http.end();
}
