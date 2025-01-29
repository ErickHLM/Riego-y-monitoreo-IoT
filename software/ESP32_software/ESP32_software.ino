#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h> // Provide the token generation process info.
#include <addons/RTDBHelper.h> // Provide the RTDB payload printing info and other helper functions.
#include <time.h>

// Pin definitions
const int lluviaSensorPin = 32;
const int relay1Pin = 13;
const int relay2Pin = 12;
const int waterLevelSensorPin = 35;
// Datos de coneccion wifi, remplasar por los de su red
const char* ssid = "leki";
const char* password = "Murrugarra.29";
IPAddress local_IP(192, 168, 0, 232);       // IP fija del ESP32
IPAddress gateway(192, 168, 0, 1);         // Puerta de enlace (IP del router)
IPAddress subnet(255, 255, 255, 0);        // Máscara de subred
IPAddress primaryDNS(8, 8, 8, 8);       // DNS de Google
IPAddress secondaryDNS(8, 8, 4, 4);     // DNS de Google

// Configurar el servidor NTP
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;  // Ajusta según tu zona horaria (ejemplo: -5 horas)
const int   daylightOffset_sec = 3600;

// Firebase credenciales
/*
#define FIREBASE_AUTH "Zih9fwFyk0UZ1t6oJBlpajSVn9EaZXBLHQ4ArlfX"
#define FIREBASE_PROJECT_ID "sistema-de-riego-iot-76aa6"
 */
#define API_KEY "AIzaSyBUOzqsE50IHq4HRxlkcT1GZp0lmlmS4mM"
#define DATABASE_URL "https://sistema-de-riego-iot-76aa6-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define USER_EMAIL "bacasiones20@gmail.com"
#define USER_PASSWORD "Murrugarra.29"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

//seccion de webserver
WebServer server(80);

// Manejador para recibir datos HTTP
String data = "";
void handleData() {
  if (server.hasArg("data")) {
    data = server.arg("data");
    Serial.println("Datos recibidos: " + data);   
    server.send(200, "text/plain", "Datos recibidos correctamente");
  } else {
    server.send(400, "text/plain", "No se enviaron datos");
  }

}

// Variable para controlar el tiempo de actualización
unsigned long sendDataPrevMillis = 0;
unsigned long count = 0;
// Función para obtener timestamp
String getTimestamp() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return "Failed to obtain time";
  }
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%dT%H:%M:%S", &timeinfo);
  return String(timeStringBuff);
}



void setup() {
  Serial.begin(115200);

  // Initialize the relay pins as outputs
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);

  // Initialize the sensor pins as inputs
  pinMode(lluviaSensorPin, INPUT);
  pinMode(waterLevelSensorPin, INPUT);

  // Configurar la hora desde el servidor NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Configurar IP fija antes de conectar a Wi-Fi
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Error al configurar IP fija");
  }
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connectando al Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connectado al Wi-Fi");
  // Esperar hasta obtener la hora
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        Serial.println("⚠️ Esperando sincronización con NTP...");
        delay(1000);
    }
    Serial.println("✅ Hora sincronizada correctamente.");

//inicializando servidor
  server.on("/sendData", HTTP_POST, handleData);  // Ruta para recibir datos
  server.begin();
  Serial.println("Servidor HTTP iniciado");

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

   // Configuración Firebase
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
  //config.timeout.socketConnection = 10 * 1000;
  Firebase.begin(&config, &auth);
    Firebase.setDoubleDigits(5);

 
}

void loop() {
  
  //resibe datos de clientes
  server.handleClient();

  if(data == NULL){
      server.handleClient();

  }else{
    // Read sensor values
    int lluviaSensorValue = analogRead(lluviaSensorPin);
    int waterLevelSensorValue = analogRead(waterLevelSensorPin);
    int humedad = String(data).toInt();
    //resibe datos de clientes
    // imprime los datos en el serial
    Serial.print("Sensor de llubia: ");
    Serial.println(lluviaSensorValue);
    Serial.print("sensor de agua: ");
    Serial.println(waterLevelSensorValue);   
    Serial.print("sensor de humedad: ");
    Serial.println(humedad);
    
     // Obtener timestamp actual
    String timestamp = getTimestamp();
    
    // Crear JSON con los datos de los sensores
    FirebaseJson sensorData;
    sensorData.set("rain/current", lluviaSensorValue);
    sensorData.set("waterLevel/current", waterLevelSensorValue);
    sensorData.set("humidity/current", humedad);
    sensorData.set("rain/history/" + timestamp + "/value", lluviaSensorValue);
    sensorData.set("rain/history/" + timestamp + "/timestamp", timestamp);
    sensorData.set("waterLevel/history/" + timestamp + "/value", waterLevelSensorValue);
    sensorData.set("waterLevel/history/" + timestamp + "/timestamp", timestamp);
    sensorData.set("/humidity/history/" + timestamp + "/value", humedad);
    sensorData.set("/humidity/history/" + timestamp +"/timestamp", timestamp);

   /*  
    if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
      {
        sendDataPrevMillis = millis(); 

        Serial.println("firebase conectado");
            count++;
      }
     */
    // Guardar datos en Firebase
       // Actualizar Firebase
    if (Firebase.updateNode(fbdo,F("/sensors"), sensorData)) {
      Serial.println("Datos de sensores actualizados en Firebase");
    } else {
      Serial.println("Error actualizando sensores: " + fbdo.errorReason());
    }
    
   
      // Control relays based on sensor values
    if (humedad > 700) {
      digitalWrite(relay1Pin, LOW); // Turn on relay 1
      Serial.println("relé 1 activado");
    } else {
      digitalWrite(relay1Pin, HIGH); // Turn off relay 1
    }

    if (waterLevelSensorValue < 2000) {
      digitalWrite(relay2Pin, LOW); // Turn on relay 2
      Serial.println("relé 2 activado");
    } else {
      digitalWrite(relay2Pin, HIGH); // Turn off relay 2
    }

    data="";
  }
   
  
  // Delay before next loop iteration
  //delay(200);
}