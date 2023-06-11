//Esta variable se usa para decidir si se corre el código de la función loop
//En función de si se está conectado o no

void horatemp() {
  static int step = 0;  // Variable para controlar el paso actual
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  uint8_t new_h = 0;
  uint8_t new_t = 0;

  // Define el intervalo de tiempo deseado en milisegundos (2,5 segundos en este caso)
  const unsigned long interval = 1500;

  // Ejecuta el código correspondiente a cada paso cuando haya pasado el intervalo de tiempo
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    switch (step) {
      case 0:
        // Paso 1: Obtener la hora actual y formando cadena de caracteres de la configuración de los horarios
        timeClient.getFormattedTime().toCharArray(nuevahora, sizeof(nuevahora));
        strcpy(hora, nuevahora);
        strcpy(horariomanana, "AM:");
        strcat(horariomanana, formateado(horamananainicio, minutomananainicio).c_str());
        strcat(horariomanana, " hasta ");
        strcat(horariomanana, formateado(horamananafin, minutomananafin).c_str());
        
        strcpy(horariotarde, "PM:");
        strcat(horariotarde, formateado(horatardeinicio, minutotardeinicio).c_str());
        strcat(horariotarde, " hasta ");
        strcat(horariotarde, formateado(horatardefin, minutotardefin).c_str());
        Serial.println("Escribiendo arreglo");
        break;

      case 1:
        // Paso 2: Leer los datos de humedad y temperatura
        new_h = dht.readHumidity();
        new_t = dht.readTemperature();

        humidity = new_h;
        temperature = new_t;
        Serial.println("Leyendo temperatura");
        break;

      case 2:
        // Paso 3: Comprobar el horario y la temperatura
        if (led_horarios() && temperature > cutemp) {
          digitalWrite(ROL, HIGH);
          Serial.println("ARRIBA");
        } else {
          digitalWrite(ROL, LOW);
          Serial.println("ABAJO");
        }

        break;
    }

    step++;  // Avanza al siguiente paso

    // Si se ha completado el último paso, reinicia el contador de pasos
    if (step > 2) {
      step = 0;
    }
  }
} 

void cambiarDireccionesHTML(const String& arc) {
  // Se abre el archivo HTML en modo lectura
  File file = SPIFFS.open(arc, "r");
  if (!file) {
    Serial.println("Error al abrir el archivo HTML");
    return;
  }

  // Se crea un archivo temporal en modo escritura
  File tempFile = SPIFFS.open("/temp.html", "w");
  if (!tempFile) {
    Serial.println("Error al crear el archivo temporal");
    file.close();
    return;
  }

  // Expresión regular para buscar direcciones IP
  std::regex ipRegex("\\b(?:\\d{1,3}\\.){3}\\d{1,3}\\b");

  // Tamaño del búfer para leer el archivo
  const size_t bufferSize = 128;
  char buffer[bufferSize];

  // Se itera sobre el archivo línea por línea
  while (file.available()) {
    // Se lee una línea del archivo
    size_t bytesRead = file.readBytesUntil('\n', buffer, bufferSize - 1);
    buffer[bytesRead] = '\0';

    // Convertimos el contenido a std::string para facilitar la manipulación
    std::string stdContent(buffer);
    std::string stdReplacement = validip;  // Reemplazamos la dirección IP encontrada en el archivo por la dirección IP válida (validip)

    // Reemplazamos todas las ocurrencias de direcciones IP en la línea actual
    std::string replacedContent = std::regex_replace(stdContent, ipRegex, stdReplacement);

    // Se escribe la línea reemplazada en el archivo temporal
    tempFile.println(replacedContent.c_str());
  }

  // Se cierran los archivos
  file.close();
  tempFile.close();

  // Se elimina el archivo original y se renombra el archivo temporal con el nombre original
  SPIFFS.remove(arc);
  SPIFFS.rename("/temp.html", arc);

  // Se imprime un mensaje indicando que las direcciones se han actualizado correctamente
  Serial.println("Las direcciones se han actualizado correctamente en el archivo HTML");
}

Espero que estos comentarios dentro del código te ayuden a comprender mejor lo que hace cada parte de la función cambiarDireccionesHTML(). Si tienes alguna pregunta adicional, no dudes en hacerla.


//Esta funcióon gestiona el formulario principal
void handleform(AsyncWebServerRequest *request) {
  // Se definen arreglos para almacenar las horas y minutos del formulario
  static uint8_t horas[4];
  static uint8_t minutos[4];

  // Se recogen todas las variables del formulario y se asignan a los arreglos correspondientes
  horas[0] = request->arg("horamañana").toInt();
  minutos[0] = request->arg("minutomañana").toInt();
  horas[1] = request->arg("horafinmañana").toInt();
  minutos[1] = request->arg("minutofinmañana").toInt();
  horas[2] = request->arg("horatardeinicio").toInt();
  minutos[2] = request->arg("minutotardeinicio").toInt();
  horas[3] = request->arg("horatardefinal").toInt();
  minutos[3] = request->arg("minutotardefinal").toInt();

  // Se recorre el arreglo y se asignan los valores correspondientes a las variables de horario
  for (int i = 0; i < 4; i++) {
    if (horas[i] != 0) {
      switch (i) {
        case 0:
          horamananainicio = horas[i];
          minutomananainicio = minutos[i];
          break;
        case 1:
          horamananafin = horas[i];
          minutomananafin = minutos[i];
          break;
        case 2:
          horatardeinicio = horas[i];
          minutotardeinicio = minutos[i];
          break;
        case 3:
          horatardefin = horas[i];
          minutotardefin = minutos[i];
          break;
      }
    }
  }

  // Se guardan los valores en la EEPROM
  epromX(true);

  // Se crea un objeto de tipo StaticJsonDocument para almacenar los datos en formato JSON
  StaticJsonDocument<200> jsonDoc;

  // Se asignan los valores a las claves correspondientes en el JSON
  jsonDoc[0]["m_inicio"] = formateado(horamananainicio, minutomananainicio);
  jsonDoc[0]["m_fin"] = formateado(horamananafin, minutomananafin);
  jsonDoc[0]["t_inicio"] = formateado(horatardeinicio, minutotardeinicio);
  jsonDoc[0]["t_fin"] = formateado(horatardefin, minutotardefin);
  jsonDoc[0]["cutemp"] = cutemp;

  // Se guarda el JSON en un archivo
  save(jsonDoc, "/datos.json");

  formEnviado = true;

  // Se envía una respuesta al cliente indicando que el formulario ha sido recibido
  request->send(200, F("text/plain"), "Formulario recibido");
}
  //Esta es la función que recoge los datos de formulario de la temperatura
  //a partir de la que se va a encender la nevera
  
 void handlenumber(AsyncWebServerRequest *request){
  //se recoge la variable del formulario
  uint8_t cotemp = request->arg("cutemp").toInt();
  //se modifica la variable global
  if (cotemp != 0){
    cutemp = cotemp;
  }
  //Se guarda la variable en la EEprom
    epromX(true);
  //Se reescribe el archivo Json
              StaticJsonDocument<200> jsonDoc;
               jsonDoc[0]["cutemp"] = cutemp;
               jsonDoc[0]["m_inicio"] = formateado(horamananainicio, minutomananainicio);
               jsonDoc[0]["m_fin"] = formateado(horamananafin, minutomananafin);
               jsonDoc[0]["t_inicio"] = formateado(horatardeinicio, minutotardeinicio);
               jsonDoc[0]["t_fin"] = formateado(horatardefin, minutotardefin);
               save(jsonDoc, "/datos.json");
               
    

    request->send(200, F("text/plain"), "Formulario recibido");
 
 }

void handleserver(AsyncWebServerRequest *request) {
  // se recoge la variable del formulario
  String servo = request->arg("sir");
  
  // se modifica la variable global
  if (servo != "") {
    strcpy(validip, servo.c_str());
  }
  
  StaticJsonDocument<200> jsonDoc;
                
  jsonDoc[0]["servidores"] = String(validip);
  jsonDoc[0]["estado"] = "ACTIVO";

   save(jsonDoc, "/server.json");
   leerserver();
  // Se guarda la variable en la EEPROM
  epromX(true);

  request->send(200, F("text/plain"), "Formulario recibido");

          cambiarDireccionesHTML("/index.html");
          cambiarDireccionesHTML("/Servidores.html");
          cambiarDireccionesHTML("/Hora.html");
          cambiarDireccionesHTML("/Temperatura.html");
}
 

//La función Initserver, gestiona todos los endpoints enviando datos para recogerlos con javascript

//Leemos de la memoria de programa los endpoints, es una técnica que ha hecho ganar una barbaridad de eficiencia al código, gracias a esto, funciona bien.
  const char temperaturaEndpoint[] PROGMEM = "/temperatura";
  const char humedadEndpoint[] PROGMEM = "/humedad";
  const char mananaEndpoint[] PROGMEM = "/manana";
  const char tardeEndpoint[] PROGMEM = "/tarde";
  const char horaEndpoint[] PROGMEM = "/hora";
  const char llorarEndpoint[] PROGMEM = "/llorar";
  const char formEndpoint[] PROGMEM = "/form";
  const char numeritoEndpoint[] PROGMEM = "/numerito";
  const char serviEndpoint[] PROGMEM = "/servi";

void InitServer()
{
  //Servimos los archivos HTML
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  server.serveStatic("/Servidores", SPIFFS, "/Servidores.html");
  server.serveStatic("/Temperatura", SPIFFS, "/Temperatura.html");
  server.serveStatic("/Hora", SPIFFS, "/Hora.html");

// Maneja la solicitud HTTP al endpoint de temperatura
server.on(temperaturaEndpoint, HTTP_GET, [](AsyncWebServerRequest *request){
  String temp = String(temperature);
  request->send_P(200, PSTR("text/plain"), temp.c_str());
});

// Maneja la solicitud HTTP al endpoint de humedad
server.on(humedadEndpoint, HTTP_GET, [](AsyncWebServerRequest *request){
  String hum = String(humidity);
  request->send_P(200, PSTR("text/plain"), hum.c_str());
});

// Maneja la solicitud HTTP al endpoint de horario de la mañana
server.on(mananaEndpoint, HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, PSTR("text/plain"), horariomanana);
});

// Maneja la solicitud HTTP al endpoint de horario de la tarde
server.on(tardeEndpoint, HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, PSTR("text/plain"), horariotarde);
});

// Maneja la solicitud HTTP al endpoint de hora actual
server.on(horaEndpoint, HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, PSTR("text/plain"), hora);
});

// Maneja la solicitud HTTP al endpoint de valor de corte de temperatura
server.on(llorarEndpoint, HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, PSTR("text/plain"), String(cutemp).c_str());
});

// Maneja la solicitud HTTP al endpoint de dirección IP del servidor
server.on(serviEndpoint, HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, PSTR("text/plain"), String(validip).c_str());
});

//Estas funciones utilizan la biblioteca AsyncWebServer para manejar las solicitudes HTTP. Cada función se asocia a un endpoint específico y utiliza la función send_P() para enviar la respuesta al cliente con el código de estado 200 (OK) y el contenido correspondiente.

//Espero que estos comentarios te ayuden a entender cómo se manejan las solicitudes HTTP en cada uno de los endpoints mencionados. Si tienes más preguntas, no dudes en hacerlas.


  server.on(formEndpoint, HTTP_POST, handleform);  // Recoge los datos del formulario principal
  server.on(numeritoEndpoint, HTTP_POST, handlenumber);  // Recoge los datos del numerito de corte
  server.on(numeritoEndpoint, HTTP_POST, handleserver);  // Recoge los datos del numerito de corte

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send_P(400, PSTR("text/plain"), PSTR("Not found"));
  });

  // Se inicia el servidor y se notifica en el serial
  server.begin();
  Serial.println(F("HTTP server started"));
}


//Esta función envía a la página 5 redes wifi detectadas
void wifis() {
  uint8_t n = WiFi.scanNetworks();

  if (n == 0) {
    // No hay redes WiFi disponibles
    server.on("/redes", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, F("text/plain"), "No hay redes WiFi disponibles.");
    });
  } else {
    // Construye el contenido HTML con la lista de redes Wi-Fi
    String html = "";
    for (uint8_t i = 0; i < 10 && i < n; ++i) {
      String ssid = WiFi.SSID(i);
      uint8_t rssi = WiFi.RSSI(i);
      // Agregar cada red en una línea separada
      html += "SSID: " + ssid + "<br>Señal: " + String(rssi) + " dBm<br>";
    }

    // Responde con el contenido HTML
    server.on("/redes", HTTP_GET, [html](AsyncWebServerRequest *request) {
      request->send(200, F("text/html"), html);
    });
  }
}

//Esta variable se usa para comprobar si el formulario ha sido enviado
bool datosnuevos = false;
//Esta función recoge el formulario del punto AP y torna datosnuevos a true
void confw(AsyncWebServerRequest *request) {
  //Se recogen las variables en Strings
  String ssidStr = request->arg("ssid");
  String passwordStr = request->arg("password");
  String sbdStr = request->arg("sbd");

  // Copia ssidStr en ssid
  strlcpy(ssid, ssidStr.c_str(), sizeof(ssid));

  // Copia passwordStr en password
  strlcpy(password, passwordStr.c_str(), sizeof(password));

  strlcpy(validip, sbdStr.c_str(), sizeof(validip));

  //Se escriben en la EEPROM
epromX(true);

     StaticJsonDocument<200> jsonDoc;
                
  jsonDoc[0]["servidores"] = sbdStr;
  jsonDoc[0]["estado"] = "ACTIVO";

   save(jsonDoc, "/server.json");


  request->send(200, F("text/plain"), "Reiniciando ESP8266");  
//Pasa a ser true y se reinicia el ESP, si se reinicara dentro de esta función no se guardarían bien las variables, 
//por eso luego se hace la comprobación
 datosnuevos = true;

}
//Esta función gestiona los endpoints del modo AP
void initAP() {

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("config.html");

  wifis();
  //se emvía la cadena de las redes wifi
  server.on("/wifi", HTTP_POST, confw);

    server.begin();
    Serial.println("HTTP server started");
}