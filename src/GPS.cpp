

/*
  GPS - FEATHER
  3v  - 3v
  GND - GND
  TX  - RX
  RX  - TX

  CASI D'USO

  Prerequisiti:
    - il programma deve essere eseguito solo quando ci si trova sullo start della pista
    - la macchina deve posizionarsi al centro della carreggiata
    - il pilota deve attendere qualche secondo prima che venga trovata la coordinata iniziale di riferimento

  Sequenza:
    1) Il GPS cerca i satelliti
    2) GPS.FIX == 1 (ho stabilito la connessione terra-satellite)
      2.1)  Prendo un numero di coordinate pari a "numCoordinates" e determino la coordinata media da cui partire.
            Definisco una circonferenza di raggio "radius" e centro la coordinata appena calcolata
      2.2)  Finito di calibrare il punto di partenza, mi salvo i valori attuali di velocità (m/s), longitudine e latitudine
      2.3)  Controllo se mi trovo all'interno del cerchio
      2.4)  Controllo se mi sto muovendo ad una velocità maggiore di "minVelocityMs" quindi sto gareggiando
      2.5)  Se:
              - sto gareggiando
              - sono all'interno della circonferenza
              - non è la prima volta che mi trovo all'interno della circonferenza
            Allora ho completato un giro
*/

#include <algorithm>
#include <Adafruit_GPS.h>
#include <math.h>

// Settings GPS
#define GPSSerial Serial1
#define GPSECHO false
Adafruit_GPS GPS(&GPSSerial);

// Settings Parameters
#define Raggio_terrestre 6371000.0f
uint32_t timer = millis();
uint8_t attemptToFindCenter = 0;                    // Indice che monitora il numero di acquisizioni per trovare la coordinata da cui partire
const uint8_t numCoordinates = 50;                  // Numero di coordinate iniziali da cui trovare la mediana
const float minVelocityKmh = 5.0f;                  // (Km/h) Velocità minima per determinare che la macchina è in movimento
const float minVelocityMs = minVelocityKmh / 3.6f;  // (m/s) Velocità minima per determinare che la macchina è in movimento
float arrayOfCoords[numCoordinates][2];             // Matrice di coordinate ausiliaria che memorizza "numCoordinates" coordinate
const float toMS = 1.852f;                          // Parametro che serve per convertire la velocità da nodi a metri al secondo
const float radius = 7.0f;

// Variables
float actualLat = 0.0f;       // Latitudine attuale
float actualLon = 0.0f;       // Longitudine attuale
float deltaLat = 0.0f;        // Distanza latitudinale
float deltaLon = 0.0f;        // Distanza longitudinale
float speed = 0.0f;           // Velocità attuale
bool isRacing = false;        // La macchina sta gareggiando? Ha una velocità superiore a "minVelocityMs"?
bool isInsideCircle = false;  // La macchina è all'interno del reaggio del cerchio?
uint8_t lapCompleted = 0;     // Numero di giri completati
bool enable = true;                // Serve come controllo aggiuntivo al conteggio dei giri. Fa si che aggiorni il contatore di solo 1 unità per giro
bool isFirstLap = true;       // Serve come controllo aggiuntivo al conteggio dei giri. Fa si che non conti la partenza come un giro completato

// Data Structure
struct Circle {
  float centerX;
  float centerY;
  float radius;
};

Circle circle;  // Definisce la struttura dati Circle che disegna la circonferenza intorno alla coordinata di partenza

void setInitCoordinates(Circle& circle);
float medianLat(float coords[numCoordinates][2]);
float medianLon(float coords[numCoordinates][2]);
void recalibrate();
float toRadians(float degree);
bool areCoordsInsideCircle(float lat, float lon, Circle circle);
float longitudinalDistance(float lon1, float lon2, float lat);
float latitudinalDistance(float lat1, float lat2);
bool isInMovement(float speed);
void saveData();
void printInformations();
float distanceBetweenCoordinates(float lat1, float lon1, float lat2, float lon2);
void printSatellites();
void printCoordinates();
void printFixAndQuality();
void printDate();
void printTime();

void setup() {
  Serial.begin(115200);
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);
  GPS.sendCommand(PMTK_API_SET_FIX_CTL_5HZ);
  GPS.sendCommand(PGCMD_ANTENNA);
  GPS.sendCommand(PMTK_ENABLE_WAAS);
  delay(1000);

  circle.radius = radius;
}

void loop()
{
  char c = GPS.read();
  if (GPS.newNMEAreceived()) {
    //Serial.print(GPS.lastNMEA());
    if (!GPS.parse(GPS.lastNMEA()))
      return;
  }

  // Aggiornamenti ogni TOT secondi/millisecondi
  if (millis() - timer > 100) {
    timer = millis();
    //printTime();
    //printDate();
    //printFienableAndQuality();
    if (GPS.fix) {

      if (attemptToFindCenter <= numCoordinates) {
        setInitCoordinates(circle);
      } else {
        actualLat = GPS.latitudeDegrees;
        actualLon = GPS.longitudeDegrees;
        speed = GPS.speed * toMS;

        isInsideCircle = areCoordsInsideCircle(actualLat, actualLon, circle);
        isRacing = isInMovement(speed);

        if (isRacing) {
          if (isInsideCircle) {
            if (enable && !isFirstLap) {
              lapCompleted++;
              enable = false;
            }
          } else {
            enable = true;
            isFirstLap = false;
          }
        }

        //printInformations();
        saveData();

        Serial.println();
      }

      //printCoordinates();
      //printSatellites();
    }
  }
}

void setInitCoordinates(Circle& circle) {
  if (attemptToFindCenter == numCoordinates) {
    circle.centerX = medianLon(arrayOfCoords);
    circle.centerY = medianLat(arrayOfCoords);
    attemptToFindCenter++;
  } else {
    arrayOfCoords[attemptToFindCenter][0] = GPS.latitudeDegrees;
    arrayOfCoords[attemptToFindCenter][1] = GPS.longitudeDegrees;
    attemptToFindCenter++;
  }
}

// Funzione per calcolare la mediana delle latitudini
float medianLat(float coords[numCoordinates][2]) {
  float latitudes[numCoordinates];
  for (int i = 0; i < numCoordinates; i++) {
    latitudes[i] = coords[i][0];
  }
  // Ordina l'array delle latitudini
  std::sort(latitudes, latitudes + numCoordinates);

  // Calcola la mediana delle latitudini
  if (numCoordinates % 2 == 0) {
    return (latitudes[numCoordinates / 2 - 1] + latitudes[numCoordinates / 2]) / 2.0f;
  } else {
    return latitudes[numCoordinates / 2];
  }
}

// Funzione per calcolare la mediana delle longitudini
float medianLon(float coords[numCoordinates][2]) {
  float longitudes[numCoordinates];
  for (int i = 0; i < numCoordinates; i++) {
    longitudes[i] = coords[i][1];
  }
  // Ordina l'array delle longitudini
  std::sort(longitudes, longitudes + numCoordinates);

  // Calcola la mediana delle longitudini
  if (numCoordinates % 2 == 0) {
    return (longitudes[numCoordinates / 2 - 1] + longitudes[numCoordinates / 2]) / 2.0f;
  } else {
    return longitudes[numCoordinates / 2];
  }
}

void recalibrate() {
  circle.centerX = actualLon;
  circle.centerY = actualLat;
}

float toRadians(float degree) {
  return degree * M_PI / 180.0;
}

/**
  Funzione per verificare se una coordinata si trova all'interno o all'esterno della circonferenza
  Calcola la distanza longitudinale e latitudinale tra il centro della circonferenza e la coordinata passata come parametro
  e verifica che le distanze sono all'interno del raggio 
**/
bool areCoordsInsideCircle(float lat, float lon, Circle circle) {
  deltaLat = fabs(latitudinalDistance(circle.centerY, lat));
  deltaLon = fabs(longitudinalDistance(circle.centerX, lon, circle.centerY));

  return (deltaLat <= circle.radius) && (deltaLon <= circle.radius);
}

// Funzione per calcolare la distanza longitudinale tra due punti sulla superficie della Terra
float longitudinalDistance(float lon1, float lon2, float lat) {
  return toRadians(lon2 - lon1) * Raggio_terrestre * cos(toRadians(lat));
}

// Funzione per calcolare la distanza latitudinale tra due punti sulla superficie della Terra
float latitudinalDistance(float lat1, float lat2) {
  return toRadians(lat2 - lat1) * Raggio_terrestre;
}

bool isInMovement(float speed) {
  if (speed <= minVelocityMs) {
    return false;
  }
  return true;
}


void saveData(){
  Serial.print(actualLat, 7);
  Serial.print(", ");
  Serial.print(actualLon, 7);
}

void printInformations() {
  printTime();
  Serial.println();
  Serial.println("Posizione attuale:      " + String(actualLon, 7) + " : " + String(actualLat, 7));
  Serial.println("Centro del cerchio:     " + String(circle.centerX, 7) + " : " + String(circle.centerY, 7));
  Serial.println("Velocità (m/s):         " + String(speed));
  Serial.println("Distanza longitudinale: " + String(deltaLon));
  Serial.println("Distanza latitudinale:  " + String(deltaLat));

  if (lapCompleted != 0) {
    Serial.println("--->Ha completato " + String(lapCompleted) + " giri.");
  }

  if (isInsideCircle) {
    Serial.println("------>Dentro il cerchio.");
  } else {
    Serial.println("------>Fuori dal cerchio.");
  }

  if (isRacing) {
    if (isInsideCircle) {
      Serial.println("------>GIRO COMPLETATO.");
    } else {
      Serial.println("------>GIRO NON COMPLETATO.");
    }
  } else {
    Serial.println("------>Veicolo fermo.");
  }
}

// NON UTILIZZATA - SCOPO AUSILIARIO
// Funzione per calcolare la distanza tra due punti sulla superficie della Terra
float distanceBetweenCoordinates(float lat1, float lon1, float lat2, float lon2) {
  float deltaLat = toRadians(lat2 - lat1);
  float deltaLon = toRadians(lon2 - lon1);

  float a = pow(sin(deltaLat / 2), 2) + cos(toRadians(lat1)) * cos(toRadians(lat2)) * pow(sin(deltaLon / 2), 2);
  float c = 2 * atan2(sqrt(a), sqrt(1 - a));
  float distance = Raggio_terrestre * c;
  return distance;
}


void printTime() {
  Serial.print("\nTime: ");
  if (GPS.hour < 10) { Serial.print('0'); }
  Serial.print(GPS.hour, DEC);
  Serial.print(':');
  if (GPS.minute < 10) { Serial.print('0'); }
  Serial.print(GPS.minute, DEC);
  Serial.print(':');
  if (GPS.seconds < 10) { Serial.print('0'); }
  Serial.print(GPS.seconds, DEC);
  Serial.print('.');
  if (GPS.milliseconds < 10) {
    Serial.print("00");
  } else if (GPS.milliseconds > 9 && GPS.milliseconds < 100) {
    Serial.print("0");
  }
}

void printDate() {
  Serial.println(GPS.milliseconds);
  Serial.print("Date: ");
  Serial.print(GPS.day, DEC);
  Serial.print('/');
  Serial.print(GPS.month, DEC);
  Serial.print("/20");
  Serial.println(GPS.year, DEC);
}

void printFixAndQuality() {
  Serial.print("Fix: ");
  Serial.print((int)GPS.fix);
  Serial.print(" quality: ");
  Serial.println((int)GPS.fixquality);
}

void printCoordinates() {
  Serial.print("Location: ");
  Serial.print(GPS.latitudeDegrees, 7);
  Serial.print(GPS.lat);
  Serial.print(", ");
  Serial.print(GPS.longitudeDegrees, 7);
  Serial.println(GPS.lon);
}

void printSatellites() {
  Serial.print("Satellites: ");
  Serial.println((int)GPS.satellites);
}