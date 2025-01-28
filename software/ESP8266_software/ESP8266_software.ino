#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "leki";
const char* password = "Murrugarra.29";
const char* serverAddress = "http://192.168.0.232";

WiFiClient client;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado a Wi-Fi");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(client,String(serverAddress)+"/sendData");  // Dirección del ESP32
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Datos a enviar
    String postData = "data=" + String(analogRead(A0));
    int httpResponseCode = http.POST(postData);
    Serial.println(postData);

    if (httpResponseCode > 0) {
      Serial.println("Respuesta del servidor: " + http.getString());
    } else {
      Serial.println("Error al enviar datos: " + String(httpResponseCode));
    }
    http.end();
     delay(10000);  // Enviar cada 5 minutos
      //ESP.deepSleep(60e6,  WAKE_RF_DEFAULT); //hibernar por 50 segundos
  }else{
    WiFi.begin(ssid, password);
  }
 
}
