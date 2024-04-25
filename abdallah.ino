#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <ESP8266WiFi.h>
#include <UbidotsESPMQTT.h>

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
const char* ssid = "VotreSSID";      // Nom de  WiFi
const char* password = "VotreMotDePasse";  // Mot de passe de  réseau WiFi

// Ubidots
#define TOKEN "BBUS-bHKWYWHW5fQU2tSMKIUXiPu7PbRUv5"  // token Ubidots
Ubidots client(TOKEN);

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

  // Connexion à Ubidots MQTT
  client.wifiConnection((char*)ssid, (char*)password); // Connexion WiFi pour Ubidots MQTT
  client.begin(); // Démarre la connexion MQTT avec Ubidots
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

  // Envoi des données à Ubidots si des coordonnées GPS valides sont disponibles
  if (gpsDataAvailable) {
    // Envoyer les données ECG à Ubidots
    client.add("ecg", ecgValue); // Ajouter la valeur de l'ECG à Ubidots

    // Envoyer les données GPS à Ubidots
    client.add("latitude", latitude);
    client.add("longitude", longitude);

    // Publier les données vers Ubidots
    client.ubidotsPublish("room"); // Publier les données dans l'emplacement "room" sur Ubidots

    gpsDataAvailable = false; // Réinitialiser le drapeau des nouvelles données GPS après l'envoi
  }

  // Attendre avant de publier à nouveau (par exemple, toutes les 5 secondes)
  delay(5000); // Envoi des données toutes les 5 secondes

  // Gestion de la boucle MQTT pour recevoir les messages
  client.loop();
}
