#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h> // Provide the token generation process info.
#include <addons/RTDBHelper.h> // Provide the RTDB payload printing info and other helper functions.
//#include <SD.h>

// Datos de coneccion wifi, remplasar por los de su red
const char* ssid = "leki";
const char* password = "Murrugarra.29";

IPAddress local_IP(192, 168, 0, 232);       // IP fija del ESP32
IPAddress gateway(192, 168, 0, 1);         // Puerta de enlace (IP del router)
IPAddress subnet(255, 255, 255, 0);        // Máscara de subred

//seccion de webserver
WebServer server(80);
int humedad_1;
String data = "";
void handleData() {
  if (server.hasArg("data")) {
    data = server.arg("data");
    Serial.println("Datos recibidos: " + data);   
    server.send(200, "text/plain", "Datos recibidos correctamente");
  } else {
    server.send(400, "text/plain", "No se enviaron datos");
  }
  //int humedad_1 = int(data);
}

// Firebase project credentials
#define FIREBASE_PROJECT_ID "your_project_id"
#define FIREBASE_API_KEY "your_api_key"
#define FIREBASE_USER_EMAIL "your_email"
#define FIREBASE_USER_PASSWORD "your_password"

// Pin definitions
const int lluviaSensorPin = 32;
const int relay1Pin = 13;
const int relay2Pin = 12;
const int waterLevelSensorPin = 35;

// Define Firebase Data object
/* FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
 */
void setup() {
  Serial.begin(115200);

  // Initialize the relay pins as outputs
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);

  // Initialize the sensor pins as inputs
/*   pinMode(lluviaSensorPin, INPUT);
  pinMode(waterLevelSensorPin, INPUT); */

  // Configurar IP fija antes de conectar a Wi-Fi
  if (!WiFi.config(local_IP, gateway, subnet)) {
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

//inicializando servidor
  server.on("/sendData", HTTP_POST, handleData);  // Ruta para recibir datos
  server.begin();
  Serial.println("Servidor HTTP iniciado");

  // Assign the project ID and API key
  /* config.api_key = FIREBASE_API_KEY;
  config.database_url = "https://" FIREBASE_PROJECT_ID ".firebaseio.com/";

  // Assign the user sign-in credentials
  auth.user.email = FIREBASE_USER_EMAIL;
  auth.user.password = FIREBASE_USER_PASSWORD;

  // Initialize Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true); */
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
    server.handleClient();
    //Serial.println(WiFi.localIP());
    // imprime los datos en el serial
    Serial.print("Sensor de llubia: ");
    Serial.println(lluviaSensorValue);
    Serial.print("sensor de agua: ");
    Serial.println(waterLevelSensorValue);   
    Serial.print("sensor de humedad: ");
    Serial.println(humedad);
    
    // Send sensor values to Firebase
    /* if (Firebase.RTDB.setInt(&fbdo, "/sensors/rain", lluviaSensorValue)) {
      Serial.println("valor del sensor de llubvia enviado correctamente");
    } else {
      Serial.println("Fallo al enviar valor de lluvia");
      Serial.println(fbdo.errorReason());
    }

    if (Firebase.RTDB.setInt(&fbdo, "/sensors/waterLevel", waterLevelSensorValue)) {
      Serial.println("Nivel de agua enviado correctamente");
    } else {
      Serial.println("Fallo al enviar nivel de agua");
      Serial.println(fbdo.errorReason());
    }
  */
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
  delay(200);
}