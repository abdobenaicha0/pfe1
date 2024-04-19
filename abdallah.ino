#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Déclaration des broches pour le module GPS NEO-6M
#define GPS_TX D8   // Broche D8 pour le TX du GPS
#define GPS_RX D7   // Broche D7 pour le RX du GPS

// Déclaration des broches pour le capteur ECG8232
int ecgPin = A0;

// Variables pour les données GPS
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);
TinyGPSPlus gps;
float latitude = 0.0;
float longitude = 0.0;
bool gpsDataAvailable = false; // Indicateur pour savoir si de nouvelles données GPS sont disponibles

// WiFi
const char* ssid = "VotreSSID";      // Nom de votre réseau WiFi
const char* password = "VotreMotDePasse";  // Mot de passe de votre réseau WiFi

// Serveur web
const char* serverUrl = "http://votresiteweb.com/endpoint";

WiFiClient wifiClient; // Objet WiFiClient

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600); // Initialisation du port série pour le GPS

  pinMode(ecgPin, INPUT);

  // Connexion au réseau WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connected to WiFi");
}

void loop() {
  // Lire les données ECG
  int ecgValue = analogRead(ecgPin);

  // Lire les données GPS
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      latitude = gps.location.lat();
      longitude = gps.location.lng();
      Serial.print("Latitude: ");
      Serial.println(latitude, 6);
      Serial.print("Longitude: ");
      Serial.println(longitude, 6);
      gpsDataAvailable = true; // Indiquer que de nouvelles données GPS sont disponibles
    }
  }

  // Envoi des données au serveur si des coordonnées GPS valides sont disponibles et si de nouvelles données sont disponibles
  if (gpsDataAvailable) {
    HTTPClient http;
    http.begin(wifiClient, serverUrl); // Utilisation de wifiClient comme premier argument

    http.addHeader("Content-Type", "application/json");

    // Création du JSON avec les données ECG et GPS
    String jsonData = "{\"ecg\": " + String(ecgValue) + ", \"latitude\": " + String(latitude, 6) + ", \"longitude\": " + String(longitude, 6) + "}";

    int httpResponseCode = http.POST(jsonData);
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String response = http.getString();
      Serial.println("Response: " + response);
    } else {
      Serial.println("Error sending data");
    }

    http.end();
    gpsDataAvailable = false; // Réinitialiser le drapeau des nouvelles données GPS après l'envoi
  }

  delay(5000); // Envoi des données toutes les 5 secondes
}
