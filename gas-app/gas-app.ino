#include "DHT.h"
#include <ESP8266WiFi.h>         // pemanggil library ESP8266WiFi
#include <ESP8266HTTPClient.h>   // pemanggil library http client ESP8266
#include <SPI.h>                 // pemanggil library SPI
#include <Wire.h>                // pemanggil library komunikasi serial
#include <WiFiClient.h>          // library client

#define DHTPIN D0                // nomor pin untuk sensor Suhu Kelembapan
#define DHTTYPE DHT11            // tipe sensor Suhu Kelembapan : DHT11
#define pinBuzzer D3
#define pinRelayFan D4           // nomor pin untuk modul Relay
#define pinMQ A0                 // nomor pin untuk sensor Gas MQ135

/* ======================================================== KALIBRASI UNTUK PENDETEKSI GAS CO2 ================================================================ */
#define RL 20     // resistor pada sensor MQ di bagian belakang
#define a 7.42    //hasil dari webplotdigizer
#define b -4.11   //hasil dari webplotdigizer
#define Ro 25.61  //hasil dari webplotdigizer
#define VC 5
/* ======================================================== KALIBRASI UNTUK PENDETEKSI GAS CO2 ================================================================ */

DHT dht(DHTPIN, DHTTYPE);        // inisiasi DHT11

// Replace with your network credentials
const char* ssid     = "Mau ? Nanya aja";   //sesuaikan punya anda
const char* password = "sipapasipagaa";   //sesuaikan punya anda

// REPLACE with your Domain name and URL path or IP address with path
const char* host = "192.168.43.218";   //sesuaikan punya anda

void setup() {
  Serial.begin(115200);
  Serial.println(F("DHT11 test!"));

  dht.begin();

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  pinMode(pinBuzzer, OUTPUT);
  pinMode(pinRelayFan, OUTPUT);
}

void loop() {
  /* ============================================================= DHT11 SENSOR PROGRAM ======================================================================= */
  float s = dht.readTemperature();
  float k = dht.readHumidity();
  Serial.print("Suhu       : ");
  Serial.print(s);
  Serial.println(" °C");
  Serial.print("Kelembapan : ");
  Serial.print(k);
  Serial.println(" %");
  /* ============================================================= DHT11 SENSOR PROGRAM ======================================================================= */

  /* ============================================================= MQ135 SENSOR PROGRAM ======================================================================= */
  int nilaimqanalog = analogRead(pinMQ);
  float VRL = nilaimqanalog * (VC / 1023.0);
  float Rs = (VC * RL / VRL) - RL;
  float ratio = Rs / Ro;
  Serial.print("Rs/Ro      : ");
  Serial.println(ratio);
  float co = a * pow((ratio), b);
  Serial.print("Nilai CO   : ");
  Serial.print(co);
  Serial.println(" ppm");
  delay(1000);
  /* ============================================================= MQ135 SENSOR PROGRAM ======================================================================= */

  if (s > 28 && co > 100.0) {
    tone(pinBuzzer, 900);
    digitalWrite(pinRelayFan, LOW);
    Serial.println("Suhu ruangan panas");
  }
  else {
    tone(pinBuzzer, 0);
    digitalWrite(pinRelayFan, HIGH);
  }

  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  String getData, Link;
  HTTPClient http;    //Declare object of class HTTPClient
  //GET Data php native
  Link = "http://" + String(host) + "/gas-app/kirimdata.php?suhu=" + String(s) + "&kelembapan=" + String(k) + "&co2=" + String(co);
  //GET Data php laravel
  //Link = "http://" + String(host) + "/gass-app/api/save?suhu=" + String(s) + "&kelembapan=" + String(k) + "&co2=" + String(co);
  http.begin(client, Link);     //Specify request destination

  int httpCode = http.GET();            //Send the request
  String payload = http.getString();    //Get the response payload

  Serial.println(payload);    //Print request response payload
  http.end();  //Close connection


  //kirim ke web
  delay(1000);
}
